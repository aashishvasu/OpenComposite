# Set the Windows kit if using MSVC
# CMake will look for the newest kit that isn't newer than this
# Required to find modern D3D12 header
if (MSVC)
	set(CMAKE_SYSTEM_VERSION 10.0.17134.0)
endif ()

cmake_minimum_required(VERSION 3.12)
project(OpenComposite)

include(scripts/pch.cmake)

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
target_include_directories(Vulkan INTERFACE libs/vulkan/Include) # TODO make this private and put the public headers elsewhere
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
target_link_libraries(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vulkan/Lib/vulkan-1.lib)
else()
target_link_libraries(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vulkan/Lib32/vulkan-1.lib)
endif()

# === OpenXR ===
# Building CMake subprojects is a real pain (IMO), so just build this here

set(XrDir libs/openxr-sdk)
set(XrDirLoader libs/openxr-sdk/src/loader)
set(XrDirCommon libs/openxr-sdk/src/common)
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
target_include_directories(OpenXR PRIVATE ${XrDirCommon} ${XrDir}/src ${XrDir}/src/external/jsoncpp/include)
target_include_directories(OpenXR PUBLIC ${XrDir}/include)
target_compile_definitions(OpenXR PRIVATE -DXR_OS_WINDOWS -DXR_USE_PLATFORM_WIN32
	-DXR_USE_GRAPHICS_API_D3D11 -DXR_USE_GRAPHICS_API_D3D12)
target_link_libraries(OpenXR PUBLIC advapi32 pathcch)

# === glm ===
# Since we used to use LibOVR's maths library, we need a replacement
# glm is an obvious choice

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE libs/glm) # No separate include directory :(

# === DrvOculus ===
add_library(DrvOculus STATIC
	DrvOculus/DrvOculus.cpp

	DrvOculus/DrvOculusCommon.h

	DrvOculus/OculusBackend.cpp
	DrvOculus/OculusBackend.h

	DrvOculus/OculusBackendSubmit.cpp
	DrvOculus/OculusBackendMirror.cpp
	DrvOculus/OculusBackendGuardian.cpp

	DrvOculus/pub/DrvOculus.h

	# Devices
	DrvOculus/OculusDevice.cpp
	DrvOculus/OculusDevice.h

	DrvOculus/OculusHMD.cpp
	DrvOculus/OculusHMD.h
)
target_include_directories(DrvOculus PUBLIC DrvOculus/pub)
target_include_directories(DrvOculus PRIVATE DrvOculus)
target_link_libraries(DrvOculus OpenVR OpenXR glm)
source_group(Public REGULAR_EXPRESSION DrvOculus/pub/*)

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
	OpenOVR/Misc/audio_override.cpp
	OpenOVR/Misc/Config.cpp
	OpenOVR/Misc/debug_helper.cpp
	OpenOVR/Misc/Haptics.cpp
	OpenOVR/Misc/xrutil.cpp
	OpenOVR/Misc/Keyboard/KeyboardLayout.cpp
	OpenOVR/Misc/Keyboard/SudoFontMeta.cpp
	OpenOVR/Misc/Keyboard/VRKeyboard.cpp
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
	OpenOVR/Misc/audio_override.h
	OpenOVR/Misc/Config.h
	OpenOVR/Misc/debug_helper.h
	OpenOVR/Misc/Haptics.h
	OpenOVR/Misc/ini.h
	OpenOVR/Misc/json/json-forwards.h
	OpenOVR/Misc/json/json.h
	OpenOVR/Misc/Keyboard/KeyboardLayout.h
	OpenOVR/Misc/Keyboard/SudoFontMeta.h
	OpenOVR/Misc/Keyboard/VRKeyboard.h
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
target_compile_definitions(OCCore PRIVATE -DSUPPORT_DX -DSUPPORT_GL -DSUPPORT_VK)
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
add_library(OCOVR SHARED
	OCOVR/dllmain.cpp

	# Resources don't get nicely included in a library file
	${OVR_RESOURCES}
)
target_link_libraries(OCOVR OCCore) # DrvOculus)
