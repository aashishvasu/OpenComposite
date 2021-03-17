# Set the Windows kit if using MSVC
# CMake will look for the newest kit that isn't newer than this
# Required to find modern D3D12 header
if (MSVC)
	set(CMAKE_SYSTEM_VERSION 10.0.17134.0)
endif ()

cmake_minimum_required(VERSION 3.12)
project(OpenComposite)

include(scripts/pch.cmake)

# Options

set(XR_VALIDATION_LAYER_PATH "" CACHE STRING "A path to set XR_API_LAYER_PATH to. Setting this will enable the debug layer")
set(XR_VALIDATION_FILE_NAME "" CACHE STRING "A filename to set XR_CORE_VALIDATION_FILE_NAME to.")

# Platform-dependent flags
# Support Vulkan on Linux instead of DirectX
if (WIN32)
	set(GRAPHICS_API_SUPPORT_FLAGS -DSUPPORT_DX -DSUPPORT_DX11 -DSUPPORT_VK -DSUPPORT_GL)
elseif (ANDROID)
	set(GRAPHICS_API_SUPPORT_FLAGS -DSUPPORT_VK) # TODO implement GLES: SUPPORT_GLES
else ()
	#  (Putting XR_STUBBED(); here so this shows up if you search for it)
	set(GRAPHICS_API_SUPPORT_FLAGS -DSUPPORT_VK -DSUPPORT_GL)
endif()

# For all projects, set the symbol visibility to private on Linux
# If we don't then if an application declares a function with the same name as an OpenComposite function
# the application's one will be used instead. This has some nasty effects - for example, when using the
# global installation both libopenvr_api.so and OpenComposite will define VR_InitInternal2. What's supposed
# to happen is that libopenvr_api calls a VRClient interface method which in turn calls OpenComposite's version
# of VR_InitInternal2. But since libopenvr_api was loaded first it's function will be used, leading to
# a stack overflow.
# In general using private symbols is a great idea, and it's something we'd want to do anyway.
# There is also -Bsymbolic set when linking the final shared library, see the bottom of this file
# FIXME the Vulkan functions still get exported, hopefully they won't fight with the application's ones.
if (WIN32)
else ()
    add_definitions(-fvisibility=hidden)
endif ()

# === Runtime Switcher ===
include_external_msproject(
	RuntimeSwitcher ${CMAKE_SOURCE_DIR}/RuntimeSwitcher/RuntimeSwitcher.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
	PLATFORM "AnyCPU"
)

# === OpenVR Headers ===
add_library(OpenVR INTERFACE)
target_include_directories(OpenVR INTERFACE SplitOpenVRHeaders) # TODO make this private and put the public headers elsewhere

# === Vulkan ===
add_library(Vulkan INTERFACE)

if (WIN32)
	# On Windows, use the bundled copy
	target_include_directories(Vulkan INTERFACE libs/vulkan/Include) # TODO make this private and put the public headers elsewhere
	if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		target_link_libraries(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vulkan/Lib/vulkan-1.lib)
	else ()
		target_link_libraries(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vulkan/Lib32/vulkan-1.lib)
	endif ()

	target_compile_definitions(Vulkan INTERFACE -DVK_USE_PLATFORM_WIN32_KHR)
elseif (ANDROID)
	# Let the system sort out the linkage, we'll just use our included headers
	target_include_directories(Vulkan INTERFACE libs/vulkan/Include)
else ()
	# On Linux, use the system copy
	find_library(Vulkan REQUIRED)
	target_include_directories(Vulkan INTERFACE ${Vulkan_INCLUDE_DIR})

	# We have to include the vulkan library, since the linker is now set to fail if we use undefined symbols
	# On Gentoo at least, it seems there's nothing in Vulkan_LIBRARIES
	if ("${Vulkan_LIBRARIES}")
		target_link_libraries(Vulkan INTERFACE ${Vulkan_LIBRARIES})
	else ()
		target_link_libraries(Vulkan INTERFACE -lvulkan)
	endif ()

	# Unfortunately we also need to link against the std::filesystem implementation
	# FIXME this hardcodes the GNU C++ library, make it work with LLVM's libc++
	target_link_libraries(Vulkan INTERFACE -lstdc++fs)
endif ()

# === OpenXR ===
# Building CMake subprojects is a real pain (IMO), so just build this here

set(XrDir libs/openxr-sdk)
set(XrDirLoader libs/openxr-sdk/src/loader)
set(XrDirCommon libs/openxr-sdk/src/common)
if (ANDROID)
	# Whatever consumes this library must then link to an OpenXR loader, such as the Oculus one
	add_library(OpenXR STATIC scripts/empty.c) # Doesn't do anything
else ()
add_library(OpenXR STATIC
	${XrDirLoader}/api_layer_interface.cpp
	${XrDirLoader}/api_layer_interface.hpp
	${XrDirLoader}/loader_core.cpp
	${XrDirLoader}/loader_instance.cpp
	${XrDirLoader}/loader_instance.hpp
	${XrDirLoader}/loader_logger.cpp
	${XrDirLoader}/loader_logger.hpp
	${XrDirLoader}/loader_logger_recorders.cpp
	${XrDirLoader}/loader_logger_recorders.hpp
	${XrDirLoader}/manifest_file.cpp
	${XrDirLoader}/manifest_file.hpp
	${XrDirLoader}/runtime_interface.cpp
	${XrDirLoader}/runtime_interface.hpp

	${XrDirLoader}/xr_generated_loader.hpp
	${XrDirLoader}/xr_generated_loader.cpp
	${XrDir}/src/xr_generated_dispatch_table.h
	${XrDir}/src/xr_generated_dispatch_table.c

	${XrDirCommon}/filesystem_utils.cpp
	${XrDirCommon}/object_info.cpp

	${XrDir}/src/external/jsoncpp/src/lib_json/json_reader.cpp
	${XrDir}/src/external/jsoncpp/src/lib_json/json_value.cpp
	${XrDir}/src/external/jsoncpp/src/lib_json/json_writer.cpp
)
endif()
target_include_directories(OpenXR PRIVATE ${XrDirCommon} ${XrDir}/src ${XrDir}/src/external/jsoncpp/include)
target_include_directories(OpenXR PUBLIC ${XrDir}/include)
# Platform-dependent flags
if (WIN32)
	target_compile_definitions(OpenXR PRIVATE -DXR_OS_WINDOWS -DXR_USE_PLATFORM_WIN32
	-DXR_USE_GRAPHICS_API_D3D11 -DXR_USE_GRAPHICS_API_D3D12 -DXR_USE_GRAPHICS_API_OPENGL)
	target_link_libraries(OpenXR PUBLIC advapi32 pathcch OpenGL32)
else()
	# TODO Turtle1331 should we include -DXR_USE_PLATFORM_(XLIB|XCB|WAYLAND) here?
	target_compile_definitions(OpenXR PRIVATE -DXR_OS_LINUX
	-DXR_USE_GRAPHICS_API_OPENGL -DXR_USE_GRAPHICS_API_VULKAN)
endif()
target_link_libraries(OpenXR PUBLIC Vulkan)

if (ANDROID)
	target_compile_definitions(OpenXR PUBLIC -DXR_USE_PLATFORM_ANDROID)
endif()

# === glm ===
# Since we used to use LibOVR's maths library, we need a replacement
# glm is an obvious choice

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE libs/glm) # No separate include directory :(

# === DrvOpenXR ===
add_library(DrvOpenXR STATIC
	DrvOpenXR/DrvOpenXR.cpp

	DrvOpenXR/XrDriverPrivate.h

	DrvOpenXR/XrBackend.cpp
	DrvOpenXR/XrBackend.h

	DrvOpenXR/XrTrackedDevice.cpp
	DrvOpenXR/XrTrackedDevice.h

	DrvOpenXR/XrHMD.cpp
	DrvOpenXR/XrHMD.h

	DrvOpenXR/XrController.cpp
	DrvOpenXR/XrController.h

	DrvOpenXR/tmp_gfx/TemporaryGraphics.cpp
	DrvOpenXR/tmp_gfx/TemporaryGraphics.h
	DrvOpenXR/tmp_gfx/TemporaryD3D11.cpp
	DrvOpenXR/tmp_gfx/TemporaryD3D11.h
	DrvOpenXR/tmp_gfx/TemporaryVk.cpp
	DrvOpenXR/tmp_gfx/TemporaryVk.h
)
target_include_directories(DrvOpenXR PUBLIC DrvOpenXR/pub)
target_include_directories(DrvOpenXR PRIVATE DrvOpenXR)
target_link_libraries(DrvOpenXR PUBLIC OpenVR OpenXR glm)
target_compile_definitions(DrvOpenXR PRIVATE ${GRAPHICS_API_SUPPORT_FLAGS} -D_CRT_SECURE_NO_WARNINGS)
source_group(Public REGULAR_EXPRESSION DrvOpenXR/pub/*)

if (NOT (XR_VALIDATION_LAYER_PATH STREQUAL ""))
	target_compile_definitions(DrvOpenXR PRIVATE -DXR_VALIDATION_LAYER_PATH=\"${XR_VALIDATION_LAYER_PATH}\")
endif ()
if (NOT (XR_VALIDATION_FILE_NAME STREQUAL ""))
	target_compile_definitions(DrvOpenXR PRIVATE -DXR_VALIDATION_FILE_NAME=\"${XR_VALIDATION_FILE_NAME}\")
endif ()

# === OCCore ===
# TODO: precompiled headers
set(OVR_RESOURCES
	OpenOVR/resources.rc
)
set(OVR_PCH_EXCLUDED
	OpenOVR/Misc/ini.c
	OpenOVR/Misc/jsoncpp.cpp
	OpenOVR/Misc/lodepng.cpp
)
add_library(OCCore STATIC
	# Use when doing PCH
	# Place it above everything else, so it gets built first
	OpenOVR/stdafx.cpp

	OpenOVR/Compositor/compositor.cpp
	OpenOVR/Compositor/dx11compositor.cpp
	OpenOVR/Compositor/dx10compositor.cpp
	OpenOVR/Compositor/dx12compositor.cpp
	OpenOVR/Compositor/glcompositor.cpp
	OpenOVR/Compositor/vkcompositor.cpp
	OpenOVR/convert.cpp
	OpenOVR/logging.cpp
	OpenOVR/linux_funcs.cpp
	OpenOVR/Misc/Config.cpp
	OpenOVR/Misc/debug_helper.cpp
	OpenOVR/Misc/Haptics.cpp
	OpenOVR/Misc/xrutil.cpp
	OpenOVR/Misc/xrmoreutils.cpp
	OpenOVR/Misc/Keyboard/KeyboardLayout.cpp
	OpenOVR/Misc/Keyboard/SudoFontMeta.cpp
	OpenOVR/Misc/Keyboard/VRKeyboard.cpp
	OpenOVR/Misc/Input/InteractionProfile.cpp
	OpenOVR/OpenOVR.cpp
	OpenOVR/Reimpl/BaseApplications.cpp
	OpenOVR/Reimpl/BaseChaperone.cpp
	OpenOVR/Reimpl/BaseChaperoneSetup.cpp
	OpenOVR/Reimpl/BaseClientCore.cpp
	OpenOVR/Reimpl/BaseCompositor.cpp
	OpenOVR/Reimpl/BaseExtendedDisplay.cpp
	OpenOVR/Reimpl/BaseInput.cpp
	OpenOVR/Reimpl/BaseInputInternal.cpp
	OpenOVR/Reimpl/BaseOverlay.cpp
	OpenOVR/Reimpl/BaseOverlayView.cpp
	OpenOVR/Reimpl/BaseRenderModels.cpp
	OpenOVR/Reimpl/BaseScreenshots.cpp
	OpenOVR/Reimpl/BaseSettings.cpp
	OpenOVR/Reimpl/BaseSystem.cpp
	OpenOVR/Reimpl/BaseMailbox.cpp
	OpenOVR/Reimpl/BaseControlPanel.cpp
	OpenOVR/Reimpl/CVRApplications.cpp
	OpenOVR/Reimpl/CVRChaperone.cpp
	OpenOVR/Reimpl/CVRChaperoneSetup.cpp
	OpenOVR/Reimpl/CVRClientCore.cpp
	OpenOVR/Reimpl/CVRCompositor.cpp
	OpenOVR/Reimpl/CVRExtendedDisplay.cpp
	OpenOVR/Reimpl/CVRInput.cpp
	OpenOVR/Reimpl/CVRInputInternal.cpp
	OpenOVR/Reimpl/CVROverlay.cpp
	OpenOVR/Reimpl/CVROverlayView.cpp
	OpenOVR/Reimpl/CVRRenderModels.cpp
	OpenOVR/Reimpl/CVRScreenshots.cpp
	OpenOVR/Reimpl/CVRSettings.cpp
	OpenOVR/Reimpl/CVRSystem.cpp
	OpenOVR/Reimpl/CVRMailbox.cpp
	OpenOVR/Reimpl/CVRControlPanel.cpp
	OpenOVR/Reimpl/stubs.gen.cpp

	# Base classes
	OpenOVR/Reimpl/BaseServerDriverHost.cpp OpenOVR/Reimpl/BaseServerDriverHost.h

	# Definitions
	OpenOVR/Reimpl/CVRServerDriverHost.cpp

	# Exclude from the PCH requirement
	${OVR_PCH_EXCLUDED}

	# Resources
	${OVR_RESOURCES}

	# Headers
	OpenOVR/BaseCommon.h
	OpenOVR/Compositor/compositor.h
	OpenOVR/convert.h
	OpenOVR/custom_types.h
	OpenOVR/logging.h
	OpenOVR/Misc/Config.h
	OpenOVR/Misc/debug_helper.h
	OpenOVR/Misc/Haptics.h
	OpenOVR/Misc/ini.h
	OpenOVR/Misc/json/json-forwards.h
	OpenOVR/Misc/json/json.h
	OpenOVR/Misc/Keyboard/KeyboardLayout.h
	OpenOVR/Misc/Keyboard/SudoFontMeta.h
	OpenOVR/Misc/Keyboard/VRKeyboard.h
	OpenOVR/Misc/Input/InteractionProfile.h
	OpenOVR/Misc/lodepng.h
	OpenOVR/Misc/ScopeGuard.h
	OpenOVR/Reimpl/BaseApplications.h
	OpenOVR/Reimpl/BaseChaperone.h
	OpenOVR/Reimpl/BaseChaperoneSetup.h
	OpenOVR/Reimpl/BaseClientCore.h
	OpenOVR/Reimpl/BaseCompositor.h
	OpenOVR/Reimpl/BaseExtendedDisplay.h
	OpenOVR/Reimpl/BaseInput.h
	OpenOVR/Reimpl/BaseInputInternal.cpp
	OpenOVR/Reimpl/BaseOverlay.h
	OpenOVR/Reimpl/BaseOverlayView.h
	OpenOVR/Reimpl/BaseRenderModels.h
	OpenOVR/Reimpl/BaseScreenshots.h
	OpenOVR/Reimpl/BaseSettings.h
	OpenOVR/Reimpl/BaseSystem.h
	OpenOVR/Reimpl/BaseMailbox.h
	OpenOVR/Reimpl/BaseControlPanel.h
	OpenOVR/Reimpl/Interfaces.h
	OpenOVR/Reimpl/static_bases.gen.h
	OpenOVR/resources.h
	OpenOVR/stdafx.h
	OpenOVR/steamvr_abi.h
	OpenOVR/targetver.h

	# Newly-added classes
	OpenOVR/Drivers/DriverManager.cpp
	OpenOVR/Drivers/DriverManager.h
	OpenOVR/Drivers/Backend.cpp
	OpenOVR/Drivers/Backend.h

	# While not actually part of OCCore, list all the OpenVR interfaces here so they are easily accessable in IDE
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRClientCore_002.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRClientCore_003.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRCompositor_017.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRMailbox_001.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRInputInternal_002.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRControlPanel_006.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRApplications_004.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRApplications_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRApplications_006.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRChaperoneSetup_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRChaperoneSetup_006.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRChaperone_003.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_012.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_013.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_014.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_015.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_016.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_018.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_019.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_020.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_021.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRCompositor_022.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRDriverManager_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRExtendedDisplay_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRInput_003.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRInput_004.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRInput_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRInput_006.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRIOBuffer_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRIOBuffer_002.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRNotifications_002.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_010.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_011.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_012.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_013.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_014.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_016.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_017.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_018.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_019.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRRenderModels_004.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRRenderModels_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRRenderModels_006.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRResources_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRScreenshots_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSettings_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSettings_002.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSpatialAnchors_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_011.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_012.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_014.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_015.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_016.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_017.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRSystem_019.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRTrackedCamera_003.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRTrackedCamera_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/openvr.h
	SplitOpenVRHeaders/OpenVR/interfaces/version.h
	SplitOpenVRHeaders/OpenVR/interfaces/vrannotation.h
	SplitOpenVRHeaders/OpenVR/interfaces/vrtypes.h

	# OpenVR Driver headers
	SplitOpenVRHeaders/OpenVR/interfaces/driver_itrackeddevicedriverprovider.h
	SplitOpenVRHeaders/OpenVR/interfaces/driver_IVRServerDriverHost_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/driver_ITrackedDeviceServerDriver_005.h
)
target_include_directories(OCCore PUBLIC OpenOVR) # TODO make this private and put the public headers elsewhere
target_include_directories(OCCore PRIVATE BundledLibs)
target_compile_definitions(OCCore PRIVATE ${GRAPHICS_API_SUPPORT_FLAGS})
target_link_libraries(OCCore OpenVR OpenXR Vulkan glm)

# Set up precompiled headers for OCCore
if (WIN32)
add_precompiled_header(OCCore ${CMAKE_SOURCE_DIR}/OpenOVR/stdafx.h EXCLUDE
	${OVR_PCH_EXCLUDED}
)
endif ()

source_group(OpenVR REGULAR_EXPRESSION SplitOpenVRHeaders/OpenVR/interfaces/*)
source_group(OpenVR\\Drivers REGULAR_EXPRESSION SplitOpenVRHeaders/OpenVR/interfaces/driver_*)
source_group(OpenVR\\Custom REGULAR_EXPRESSION SplitOpenVRHeaders/OpenVR/custom_interfaces/*)
source_group(Interfaces REGULAR_EXPRESSION OpenOVR/Reimpl/Base*)
source_group(Interfaces\\Declarations REGULAR_EXPRESSION OpenOVR/Reimpl/CVR*)

# === OCOVR ===
if (WIN32)
	set(OCOVR_PLAT_SRC windows.cpp)
else ()
	set(OCOVR_PLAT_SRC linux.cpp)
endif ()
set(OCOVR_SRC_ALL
	OCOVR/${OCOVR_PLAT_SRC}
	OCOVR/openvr_api.cpp

	# Resources don't get nicely included in a library file
	${OVR_RESOURCES}
)

# On Android, build a static library so it can be linked into the target application
# This is required since we can't build the openxr loader on android.
if (ANDROID)
	add_library(OCOVR STATIC ${OCOVR_SRC_ALL})
else ()
	add_library(OCOVR SHARED ${OCOVR_SRC_ALL})
endif ()

# Require that all symbols are defined on Linux - this is always done on Windows
# Also on Linux, put all the static libraries in a group to avoid ordering problems
if (NOT WIN32)
	target_link_options(OCOVR PRIVATE -Wl,--no-undefined)
	target_link_libraries(OCOVR PUBLIC -Wl,--start-group OCCore DrvOpenXR -Wl,--end-group)
else ()
	# On Windows this isn't a problem since the linker knows how to figure this stuff out
	target_link_libraries(OCOVR OCCore DrvOpenXR)
endif ()

# If we're on Linux, compile everything as position-independent
if (WIN32)
else ()
	target_compile_options(DrvOpenXR PRIVATE -fPIC)
	target_compile_options(OpenXR PRIVATE -fPIC)
	target_compile_options(OCCore PRIVATE -fPIC)
endif ()

# The -Bsymbolic makes this work on exported functions - otherwise those will still use the first-defined one.
# See the comment at the top about -fvisibility=hidden
if (WIN32)
else ()
	target_link_options(OCOVR PRIVATE -Wl,-Bsymbolic -Wl,-Bsymbolic-functions)

	# Oh, and we need dlopen
	target_link_libraries(OCOVR PUBLIC -ldl)
endif ()

# Required because the compositor header needs the platform-specific structures enabled via defines
target_compile_definitions(OCOVR PRIVATE ${GRAPHICS_API_SUPPORT_FLAGS})
