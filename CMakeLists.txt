# Set the Windows kit if using MSVC
# CMake will look for the newest kit that isn't newer than this
# Required to find modern D3D12 header
if (MSVC)
	set(CMAKE_SYSTEM_VERSION 10.0.17134.0)
endif ()

cmake_minimum_required(VERSION 3.16)
project(OpenComposite)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options

set(XR_VALIDATION_LAYER_PATH "" CACHE STRING "A path to set XR_API_LAYER_PATH to. Setting this will enable the debug layer")
set(XR_VALIDATION_FILE_NAME "" CACHE STRING "A filename to set XR_CORE_VALIDATION_FILE_NAME to.")

option(USE_SYSTEM_OPENXR "Try using system installation of OpenXR if available" OFF)
option(USE_SYSTEM_GLM "Try using system installation of glm if available" OFF)
option(ERROR_ON_WARNING "Set all warnings to be errors" OFF)

# Directory for generated files, those being split headers and stubs
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# Platform-dependent flags
# Support Vulkan on Linux instead of DirectX
if (WIN32)
	set(GRAPHICS_API_SUPPORT_FLAGS -DSUPPORT_DX -DSUPPORT_DX11 -DSUPPORT_DX12 -DSUPPORT_VK -DSUPPORT_GL)
elseif (ANDROID)
	set(GRAPHICS_API_SUPPORT_FLAGS -DSUPPORT_VK -DSUPPORT_GLES)
else ()
	set(GRAPHICS_API_SUPPORT_FLAGS -DSUPPORT_VK -DSUPPORT_GL)
endif()

set(FORCE_COLORED_OUTPUT            OFF CACHE BOOL "Always produce ANSI-colored output (GNU/Clang only).")

if(FORCE_COLORED_OUTPUT)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif()
endif()

if (MSVC)
	add_compile_definitions(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING _CRT_SECURE_NO_WARNINGS)
	add_compile_options(/W3
		# ignored errors
		/wd4305 # truncation from double to float
		/wd4267 # size_t to uint32_t
	)
	set(ERROR_ON_WARNING_FLAG /WX)
else ()
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
    add_definitions(-fvisibility=hidden)
	add_compile_options(-Wall -Wextra -Wpedantic -pedantic-errors -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-security)
	set(ERROR_ON_WARNING_FLAG -Werror)
endif ()
if (ERROR_ON_WARNING)
	add_compile_options(${ERROR_ON_WARNING_FLAG})
endif()

# === Runtime Switcher ===
include_external_msproject(
	RuntimeSwitcher ${CMAKE_SOURCE_DIR}/RuntimeSwitcher/RuntimeSwitcher.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
	PLATFORM "AnyCPU"
)

# === OpenVR Headers ===
add_library(OpenVR INTERFACE)
target_include_directories(OpenVR INTERFACE OpenVRHeaders) # TODO make this private and put the public headers elsewhere

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
	find_library(Vulkan vulkan REQUIRED)
	target_include_directories(Vulkan INTERFACE ${Vulkan_INCLUDE_DIR})

	# We have to include the vulkan library, since the linker is now set to fail if we use undefined symbols
	# On Gentoo at least, it seems there's nothing in Vulkan_LIBRARIES
	if ("${Vulkan_LIBRARIES}")
		target_link_libraries(Vulkan INTERFACE ${Vulkan_LIBRARIES})
	else ()
		target_link_libraries(Vulkan INTERFACE -lvulkan)
	endif ()

endif ()

# === OpenXR ===
add_compile_definitions(XR_USE_GRAPHICS_API_OPENGL XR_USE_GRAPHICS_API_VULKAN)
# Platform-dependent flags
if (WIN32)
	add_compile_definitions(XR_OS_WINDOWS XR_USE_PLATFORM_WIN32
		XR_USE_GRAPHICS_API_D3D11 XR_USE_GRAPHICS_API_D3D12)
else()
	add_compile_definitions(XR_OS_LINUX XR_USE_PLATFORM_XLIB)
endif()

if (USE_SYSTEM_OPENXR)
	find_package(OpenXR)
	set(XrLib OpenXR::openxr_loader)

	find_package(jsoncpp)
	set(jsoncppLib JsonCpp::JsonCpp)
endif()

if (NOT OpenXR_FOUND OR NOT jsoncpp_FOUND)
	set(XrDir libs/openxr-sdk)

	add_library(jsoncpp STATIC
		${XrDir}/src/external/jsoncpp/src/lib_json/json_reader.cpp
		${XrDir}/src/external/jsoncpp/src/lib_json/json_value.cpp
		${XrDir}/src/external/jsoncpp/src/lib_json/json_writer.cpp
	)

	target_include_directories(jsoncpp PUBLIC ${XrDir}/src/external/jsoncpp/include)
	if (NOT MSVC)
		target_compile_options(jsoncpp PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>)
	endif()

	set(jsoncppLib jsoncpp)

	# Building CMake subprojects is a real pain (IMO), so just build this here
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

	)
	endif()
	target_include_directories(OpenXR PRIVATE ${XrDirCommon} ${XrDir}/src)
	target_include_directories(OpenXR PUBLIC ${XrDir}/include)
	# Platform-dependent flags
	if (WIN32)
		target_link_libraries(OpenXR PUBLIC advapi32 pathcch OpenGL32)
	else()
		target_link_libraries(OpenXR PUBLIC -ldl)
	endif()
	target_link_libraries(OpenXR PUBLIC Vulkan jsoncpp)
	set(XrLib OpenXR)
endif()

# === glm ===
# Since we used to use LibOVR's maths library, we need a replacement
# glm is an obvious choice

if (USE_SYSTEM_GLM)
	find_package(glm)
	set(glmLib glm::glm)
endif()

if (NOT glm_FOUND)
	add_library(glm INTERFACE)
	target_include_directories(glm INTERFACE libs/glm) # No separate include directory :(
	if (NOT MSVC)
		target_compile_options(glm INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>)
	endif()
	set(glmLib glm)
endif()

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

	DrvOpenXR/XrGenericTracker.cpp
	DrvOpenXR/XrGenericTracker.h

	DrvOpenXR/tmp_gfx/TemporaryGraphics.cpp
	DrvOpenXR/tmp_gfx/TemporaryGraphics.h
	DrvOpenXR/tmp_gfx/TemporaryD3D11.cpp
	DrvOpenXR/tmp_gfx/TemporaryD3D11.h
	DrvOpenXR/tmp_gfx/TemporaryVk.cpp
	DrvOpenXR/tmp_gfx/TemporaryVk.h
	# Ensuring vrtypes.h exists should mean the rest of our generated interfaces exists
	${GENERATED_DIR}/interfaces/vrtypes.h
	${GENERATED_DIR}/static_bases.gen.h
)
target_include_directories(DrvOpenXR PUBLIC DrvOpenXR/pub ${CMAKE_BINARY_DIR})
target_include_directories(DrvOpenXR PRIVATE DrvOpenXR OpenOVR)
target_link_libraries(DrvOpenXR PUBLIC OpenVR ${XrLib} ${jsoncppLib} ${glmLib})
if (NOT WIN32 AND NOT ANDROID)
	target_link_libraries(DrvOpenXR PRIVATE X11) # for XFree()
endif ()
target_compile_definitions(DrvOpenXR PRIVATE ${GRAPHICS_API_SUPPORT_FLAGS})
source_group(Public REGULAR_EXPRESSION DrvOpenXR/pub/*)

if (NOT (XR_VALIDATION_LAYER_PATH STREQUAL ""))
	target_compile_definitions(DrvOpenXR PRIVATE -DXR_VALIDATION_LAYER_PATH=\"${XR_VALIDATION_LAYER_PATH}\")
endif ()
if (NOT (XR_VALIDATION_FILE_NAME STREQUAL ""))
	target_compile_definitions(DrvOpenXR PRIVATE -DXR_VALIDATION_FILE_NAME=\"${XR_VALIDATION_FILE_NAME}\")
endif ()

# === OCCore ===
# TODO: precompiled headers
if (WIN32)
	set(OVR_RESOURCES
		OpenOVR/resources.rc
	)
else ()
	enable_language(ASM)
	set_property(SOURCE OpenOVR/Misc/resources_linux.s APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp")
	set_property(SOURCE OpenOVR/Misc/resources_linux.s SET PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}")
	set(OVR_RESOURCES
		OpenOVR/Misc/resources_linux.s
	)
endif ()
set(OVR_PCH_EXCLUDED
	OpenOVR/Misc/ini.c
	OpenOVR/Misc/lodepng.cpp
)

# Generate version string from git revision, or supplied OC_VERSION setting
add_custom_target(VersionHeader
	COMMENT "Generating version.h..."
	COMMAND ${CMAKE_COMMAND}
		-DIN=${CMAKE_SOURCE_DIR}/scripts/version.h.in
		-DOUT=${GENERATED_DIR}/version.h
		-DOC_VERSION=${OC_VERSION}
		-P ${CMAKE_SOURCE_DIR}/scripts/cmake/genversion.cmake
	BYPRODUCTS ${GENERATED_DIR}/version.h
	VERBATIM
)
add_library(OCCore STATIC
	# This file is used to generate the precompiled headers
	OpenOVR/stdafx.cpp

	OpenOVR/Compositor/compositor.cpp
	OpenOVR/Compositor/dx11compositor.cpp
	OpenOVR/Compositor/dx10compositor.cpp
	OpenOVR/Compositor/dx12compositor.cpp
	OpenOVR/Compositor/glcompositor.cpp
	OpenOVR/Compositor/glescompositor.cpp
	OpenOVR/Compositor/vkcompositor.cpp
	OpenOVR/convert.cpp
	OpenOVR/logging.cpp
	OpenOVR/linux_funcs.cpp
	OpenOVR/Misc/Config.cpp
	OpenOVR/Misc/debug_helper.cpp
	OpenOVR/Misc/xrutil.cpp
	OpenOVR/Misc/xrmoreutils.cpp
	OpenOVR/Misc/Keyboard/KeyboardLayout.cpp
	OpenOVR/Misc/Keyboard/SudoFontMeta.cpp
	OpenOVR/Misc/Keyboard/VRKeyboard.cpp
	OpenOVR/Misc/Input/InteractionProfile.cpp
	OpenOVR/Misc/Input/OculusInteractionProfile.cpp
	OpenOVR/Misc/Input/KhrSimpleInteractionProfile.cpp
	OpenOVR/Misc/Input/ViveInteractionProfile.cpp
	OpenOVR/Misc/Input/IndexControllerInteractionProfile.cpp
	OpenOVR/Misc/Input/HolographicInteractionProfile.cpp
	OpenOVR/Misc/Input/ReverbG2InteractionProfile.cpp
	OpenOVR/Misc/Input/ViveTrackerInteractionProfile.cpp
	OpenOVR/OpenOVR.cpp
	OpenOVR/Reimpl/BaseApplications.cpp
	OpenOVR/Reimpl/BaseChaperone.cpp
	OpenOVR/Reimpl/BaseChaperoneSetup.cpp
	OpenOVR/Reimpl/BaseClientCore.cpp
	OpenOVR/Reimpl/BaseCompositor.cpp
	OpenOVR/Reimpl/BaseExtendedDisplay.cpp
	OpenOVR/Reimpl/BaseInput.cpp
	OpenOVR/Reimpl/BaseInput_Hand.cpp
	OpenOVR/Reimpl/BaseInputInternal.cpp
	OpenOVR/Reimpl/BaseOverlay.cpp
	OpenOVR/Reimpl/BaseOverlayView.cpp
	OpenOVR/Reimpl/BaseRenderModels.cpp
	OpenOVR/Reimpl/BaseScreenshots.cpp
	OpenOVR/Reimpl/BaseSettings.cpp
	OpenOVR/Reimpl/BaseSystem.cpp
	OpenOVR/Reimpl/BaseMailbox.cpp
	OpenOVR/Reimpl/BaseControlPanel.cpp
	OpenOVR/Reimpl/BaseHeadsetView.cpp
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
	OpenOVR/Reimpl/CVRHeadsetView.cpp
	${GENERATED_DIR}/stubs.gen.cpp

	# Exclude from the PCH requirement
	${OVR_PCH_EXCLUDED}

	# Headers
	${GENERATED_DIR}/version.h
	OpenOVR/BaseCommon.h
	OpenOVR/Compositor/compositor.h
	OpenOVR/convert.h
	OpenOVR/custom_types.h
	OpenOVR/logging.h
	OpenOVR/Misc/Config.h
	OpenOVR/Misc/debug_helper.h
	OpenOVR/Misc/ini.h
	OpenOVR/Misc/Keyboard/KeyboardLayout.h
	OpenOVR/Misc/Keyboard/SudoFontMeta.h
	OpenOVR/Misc/Keyboard/VRKeyboard.h
	OpenOVR/Misc/Input/InteractionProfile.h
	OpenOVR/Misc/Input/OculusInteractionProfile.h
	OpenOVR/Misc/Input/KhrSimpleInteractionProfile.h
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
	OpenOVR/Reimpl/BaseHeadsetView.h
	OpenOVR/Reimpl/Interfaces.h
	${GENERATED_DIR}/static_bases.gen.h
	OpenOVR/resources.h
	OpenOVR/stdafx.h
	OpenOVR/steamvr_abi.h
	OpenOVR/targetver.h

	# Newly-added classes
	OpenOVR/Drivers/Backend.cpp
	OpenOVR/Drivers/Backend.h

	# While not actually part of OCCore, list all the OpenVR interfaces here so they are easily accessable in IDE
	OpenVRHeaders/custom_interfaces/IVRClientCore_002.h
	OpenVRHeaders/custom_interfaces/IVRClientCore_003.h
	OpenVRHeaders/custom_interfaces/IVRCompositor_017.h
	OpenVRHeaders/custom_interfaces/IVRMailbox_001.h
	OpenVRHeaders/custom_interfaces/IVRInputInternal_002.h
	OpenVRHeaders/custom_interfaces/IVRControlPanel_006.h

	RuntimeExtensions/openxr_extension_helpers.h
	RuntimeExtensions/XR_MNDX_xdev_space.h
	# Ensuring vrtypes.h exists should mean the rest of our generated interfaces exists
	${GENERATED_DIR}/interfaces/vrtypes.h
)
target_include_directories(OCCore PUBLIC OpenOVR ${CMAKE_BINARY_DIR})  # TODO make this private and put the public headers elsewhere
target_include_directories(OCCore PRIVATE BundledLibs OpenVRHeaders)
target_compile_definitions(OCCore PRIVATE ${GRAPHICS_API_SUPPORT_FLAGS})

target_link_libraries(OCCore OpenVR Vulkan ${XrLib} ${glmLib})

if (NOT WIN32 AND NOT ANDROID)
	find_package(OpenGL REQUIRED) # for glGetError()
	target_link_libraries(OCCore ${OPENGL_LIBRARIES})
endif ()

# Set up precompiled headers for OCCore
target_precompile_headers(OCCore PRIVATE ${CMAKE_SOURCE_DIR}/OpenOVR/stdafx.h)
set_source_files_properties(${OVR_PCH_EXCLUDED} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

source_group(OpenVR REGULAR_EXPRESSION ${GENERATED_DIR}/interfaces)
source_group(OpenVR\\Drivers REGULAR_EXPRESSION ${GENERATED_DIR}/interfaces/driver_*)
source_group(OpenVR\\Custom REGULAR_EXPRESSION OpenVRHeaders/custom_interfaces/*)
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
	OCOVR/openvr_api_0_9_12.cpp

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
	target_link_libraries(OCOVR PRIVATE -Wl,--no-undefined)
	target_link_libraries(OCOVR PUBLIC -Wl,--start-group OCCore DrvOpenXR -Wl,--end-group)
else ()
	# On Windows this isn't a problem since the linker knows how to figure this stuff out
	target_link_libraries(OCOVR OCCore DrvOpenXR)
endif ()

# If we're on Linux, compile everything as position-independent
if (WIN32)
else ()
	target_compile_options(DrvOpenXR PRIVATE -fPIC)
	if (NOT OpenXR_FOUND OR NOT jsoncpp_FOUND)
		target_compile_options(OpenXR PRIVATE -fPIC)
		target_compile_options(jsoncpp PRIVATE -fPIC)
	endif ()
	target_compile_options(OCCore PRIVATE -fPIC)
endif ()

# The -Bsymbolic makes this work on exported functions - otherwise those will still use the first-defined one.
# See the comment at the top about -fvisibility=hidden
if (WIN32)
else ()
	target_link_options(OCOVR PRIVATE -Wl,-Bsymbolic -Wl,-Bsymbolic-functions)

	# Ensure all required symbols are actually found, don't produce a binary that can't be loaded
	target_link_options(OCOVR PRIVATE -z defs)

	# Oh, and we need dlopen
	target_link_libraries(OCOVR PUBLIC -ldl)
endif ()

# needed for generated files
target_include_directories(OCOVR PRIVATE OpenOVR/Reimpl)

# Required because the compositor header needs the platform-specific structures enabled via defines
target_compile_definitions(OCOVR PRIVATE ${GRAPHICS_API_SUPPORT_FLAGS})

find_package(Python REQUIRED Interpreter)

# glob for all openvr headers
file(GLOB openvr-headers CONFIGURE_DEPENDS LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/OpenVRHeaders/openvr-*)

# glob for stub generator libraries
file(GLOB stub-deps CONFIGURE_DEPENDS LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/scripts/stubs/*)

# Command for running header splitting script
add_custom_command(
	OUTPUT ${GENERATED_DIR}/interfaces/vrtypes.h
	COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/split_headers.py ${CMAKE_SOURCE_DIR}/OpenVRHeaders ${GENERATED_DIR}
	DEPENDS ${CMAKE_SOURCE_DIR}/scripts/split_headers.py ${openvr-headers}
	COMMENT "Splitting headers..."
	)

# Command for running stub generator
add_custom_command(
	OUTPUT ${GENERATED_DIR}/stubs.gen.cpp ${GENERATED_DIR}/static_bases.gen.h
	COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/stubs.py ${CMAKE_SOURCE_DIR}/OpenOVR/Reimpl ${CMAKE_SOURCE_DIR}/OpenVRHeaders ${GENERATED_DIR}
	DEPENDS ${CMAKE_SOURCE_DIR}/scripts/stubs.py ${stub-deps} ${GENERATED_DIR}/interfaces/vrtypes.h
	COMMENT "Generating stubs..."
	)

# Allows clean target to delete generated files too
set_property(
	TARGET OCCore
	APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${GENERATED_DIR};${CMAKE_BINARY_DIR}/bin)

if ( CMAKE_SIZEOF_VOID_P EQUAL 8) # 64 bit build - void pointer is 8 bits on 64 bit
	set(is_64bit_build true)
else()
	set(is_64bit_build false)
endif()

# Move OCOVR to appropriate directory expected by OpenVR and rename it to vrclient (or vrclient_x64)
# This enables marking the build directory as the OpenVR runtime path.
# For example, you can set the VR_OVERRIDE variable to the build dir (VR_OVERRIDE=<path to build dir>) when launching an app.
# see VR_LoadHmdSystemInternal() from openvr_api_public.cpp in the OpenVR repo for the expected paths.
if (WIN32)
	set(VRCLIENT_DIR bin)
	if ( is_64bit_build )
		set(VRCLIENT_NAME vrclient_x64)
	else()
		set(VRCLIENT_NAME vrclient)
	endif()
else ()
	set(VRCLIENT_NAME vrclient)
	if ( is_64bit_build )
		set(VRCLIENT_DIR bin/linux64)
	else()
		set(VRCLIENT_DIR bin)
	endif()
endif ()

# Windows puts DLLs in RUNTIME_OUTPUT_DIRECTORY for some reason
set_target_properties(OCOVR PROPERTIES
	PREFIX ""
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${VRCLIENT_DIR}
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${VRCLIENT_DIR}
	LIBRARY_OUTPUT_NAME ${VRCLIENT_NAME}
	RUNTIME_OUTPUT_NAME ${VRCLIENT_NAME})

# ensure output directory exists
get_target_property(output_dir OCOVR LIBRARY_OUTPUT_DIRECTORY)
add_custom_command(TARGET OCOVR
	PRE_LINK COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir})

# Installation
include(GNUInstallDirs)
install(TARGETS OCOVR
	LIBRARY
	DESTINATION "${PROJECT_NAME}/${VRCLIENT_DIR}"
	COMPONENT Runtime
)
