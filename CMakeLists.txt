# Set the Windows kit if using MSVC
# CMake will look for the newest kit that isn't newer than this
# Required to find modern D3D12 header
if(MSVC)
	set(CMAKE_SYSTEM_VERSION 10.0.17134.0)
endif()

cmake_minimum_required(VERSION 3.12)
project(OpenComposite)

# === Runtime Switcher ===
include_external_msproject(
	RuntimeSwitcher ${CMAKE_SOURCE_DIR}/RuntimeSwitcher/RuntimeSwitcher.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
	PLATFORM "AnyCPU"
)

# === OpenVR Headers ===
add_library(OpenVR INTERFACE)
target_include_directories(OpenVR INTERFACE SplitOpenVRHeaders) # TODO make this private and put the public headers elsewhere

# === LibOVR ===
set(OVRDIR libs/libovr/OculusSDK/LibOVR)
add_library(ovr STATIC
	${OVRDIR}/Shim/OVR_CAPI_Prototypes.h
	${OVRDIR}/Shim/OVR_CAPI_Util.cpp
	${OVRDIR}/Shim/OVR_CAPIShim.c
	${OVRDIR}/Shim/OVR_StereoProjection.cpp
)
target_include_directories(ovr PUBLIC ${OVRDIR}/Include)

# === LibOVRKernel ===
set(OVRKDIR libs/libovr/OculusSDK/LibOVRKernel)
set(OVRKSDIR ${OVRKDIR}/Src/Util/Shaders)
set(OVR_SHADERS
	${OVRKSDIR}/Blt_ps.hlsl
	${OVRKSDIR}/Blt_ps_ms2.hlsl
	${OVRKSDIR}/Blt_vs.hlsl
)
add_library(ovrkernel STATIC
	${OVRKDIR}/../Logging/src/internal/Logging_Tools.cpp
	${OVRKDIR}/../Logging/src/Logging_Library.cpp
	${OVRKDIR}/../Logging/src/Logging_OutputPlugins.cpp
	${OVRKDIR}/Src/GL/CAPI_GLE.cpp
	${OVRKDIR}/Src/Kernel/OVR_Alg.cpp
	${OVRKDIR}/Src/Kernel/OVR_Allocator.cpp
	${OVRKDIR}/Src/Kernel/OVR_Atomic.cpp
	${OVRKDIR}/Src/Kernel/OVR_Callbacks.cpp
	${OVRKDIR}/Src/Kernel/OVR_CRC32.cpp
	${OVRKDIR}/Src/Kernel/OVR_DebugHelp.cpp
	${OVRKDIR}/Src/Kernel/OVR_Error.cpp
	${OVRKDIR}/Src/Kernel/OVR_File.cpp
	${OVRKDIR}/Src/Kernel/OVR_FileFILE.cpp
	${OVRKDIR}/Src/Kernel/OVR_JSON.cpp
	${OVRKDIR}/Src/Kernel/OVR_Log.cpp
	${OVRKDIR}/Src/Kernel/OVR_mach_exc_OSX.c
	${OVRKDIR}/Src/Kernel/OVR_Rand.cpp
	${OVRKDIR}/Src/Kernel/OVR_RefCount.cpp
	${OVRKDIR}/Src/Kernel/OVR_SharedMemory.cpp
	${OVRKDIR}/Src/Kernel/OVR_Std.cpp
	${OVRKDIR}/Src/Kernel/OVR_String.cpp
	${OVRKDIR}/Src/Kernel/OVR_String_FormatUtil.cpp
	${OVRKDIR}/Src/Kernel/OVR_String_PathUtil.cpp
	${OVRKDIR}/Src/Kernel/OVR_SysFile.cpp
	${OVRKDIR}/Src/Kernel/OVR_System.cpp
	${OVRKDIR}/Src/Kernel/OVR_ThreadsPthread.cpp
	${OVRKDIR}/Src/Kernel/OVR_ThreadsWinAPI.cpp
	${OVRKDIR}/Src/Kernel/OVR_Timer.cpp
	${OVRKDIR}/Src/Kernel/OVR_UTF8Util.cpp
	${OVRKDIR}/Src/Util/Util_D3D11_Blitter.cpp
	${OVRKDIR}/Src/Util/Util_Direct3D.cpp
	${OVRKDIR}/Src/Util/Util_GL_Blitter.cpp
	${OVRKDIR}/Src/Util/Util_ImageWindow.cpp
	${OVRKDIR}/Src/Util/Util_LongPollThread.cpp
	${OVRKDIR}/Src/Util/Util_SystemGUI.cpp
	${OVRKDIR}/Src/Util/Util_SystemInfo.cpp
	${OVRKDIR}/Src/Util/Util_Watchdog.cpp

	${OVR_SHADERS}
)
target_include_directories(ovrkernel PUBLIC ${OVRKDIR}/Src)
target_include_directories(ovrkernel PRIVATE ${OVRKDIR}/../Logging/include)
target_link_libraries(ovrkernel ovr)

# Shader configuration
set(OVR_SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen_shaders")
set(OVR_SHADER_FN "${OVR_SHADER_DIR}/Shaders/%(Filename).h")
target_include_directories(ovrkernel PRIVATE ${OVR_SHADER_DIR})

set_property(SOURCE ${OVR_SHADERS} PROPERTY VS_SHADER_VARIABLE_NAME "%(Filename)")
set_property(SOURCE ${OVR_SHADERS} PROPERTY VS_SHADER_OUTPUT_HEADER_FILE ${OVR_SHADER_FN})

set_property(SOURCE ${OVRKSDIR}/Blt_ps.hlsl PROPERTY VS_SHADER_MODEL 4.0)
set_property(SOURCE ${OVRKSDIR}/Blt_ps_ms2.hlsl PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${OVRKSDIR}/Blt_ps_vs.hlsl PROPERTY VS_SHADER_MODEL 5.0)

set_property(SOURCE ${OVRKSDIR}/Blt_ps.hlsl PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${OVRKSDIR}/Blt_ps_ms2.hlsl PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${OVRKSDIR}/Blt_vs.hlsl PROPERTY VS_SHADER_TYPE Vertex)

# === OCCore ===
# TODO: precompiled headers
set(OVR_RESOURCES
	OpenOVR/resources.rc
)
add_library(OCCore STATIC
	OpenOVR/API/OCBaseSystem.cpp
	OpenOVR/Compositor/compositor.cpp
	OpenOVR/Compositor/dx11compositor.cpp
	OpenOVR/Compositor/dx10compositor.cpp
	OpenOVR/Compositor/dx12compositor.cpp
	OpenOVR/Compositor/glcompositor.cpp
	OpenOVR/Compositor/vkcompositor.cpp
	OpenOVR/convert.cpp
	OpenOVR/libovr_wrapper.cpp
	OpenOVR/logging.cpp
	OpenOVR/Misc/audio_override.cpp
	OpenOVR/Misc/Config.cpp
	OpenOVR/Misc/debug_helper.cpp
	OpenOVR/Misc/Haptics.cpp
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
	OpenOVR/Reimpl/BaseOverlay.cpp
	OpenOVR/Reimpl/BaseRenderModels.cpp
	OpenOVR/Reimpl/BaseScreenshots.cpp
	OpenOVR/Reimpl/BaseSettings.cpp
	OpenOVR/Reimpl/BaseSystem.cpp
	OpenOVR/Reimpl/CVRApplications.cpp
	OpenOVR/Reimpl/CVRChaperone.cpp
	OpenOVR/Reimpl/CVRChaperoneSetup.cpp
	OpenOVR/Reimpl/CVRClientCore.cpp
	OpenOVR/Reimpl/CVRCompositor.cpp
	OpenOVR/Reimpl/CVRExtendedDisplay.cpp
	OpenOVR/Reimpl/CVRInput.cpp
	OpenOVR/Reimpl/CVROCSystem.cpp
	OpenOVR/Reimpl/CVROverlay.cpp
	OpenOVR/Reimpl/CVRRenderModels.cpp
	OpenOVR/Reimpl/CVRScreenshots.cpp
	OpenOVR/Reimpl/CVRSettings.cpp
	OpenOVR/Reimpl/CVRSystem.cpp
	OpenOVR/Reimpl/stubs.gen.cpp

	# Base classes
	OpenOVR/Reimpl/BaseServerDriverHost.cpp OpenOVR/Reimpl/BaseServerDriverHost.h

	# Definitions
	OpenOVR/Reimpl/CVRServerDriverHost.cpp

	# Use when doing PCH
	OpenOVR/stdafx.cpp

	# Exclude from the PCH requirement
	OpenOVR/Misc/ini.c
	OpenOVR/Misc/jsoncpp.cpp
	OpenOVR/Misc/lodepng.cpp

	# Resources
	${OVR_RESOURCES}

	# Headers
	OpenOVR/API/ISystem_001.h
	OpenOVR/API/OCBaseSystem.h
	OpenOVR/BaseCommon.h
	OpenOVR/Compositor/compositor.h
	OpenOVR/convert.h
	OpenOVR/custom_types.h
	OpenOVR/libovr_wrapper.h
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
	OpenOVR/Reimpl/BaseOverlay.h
	OpenOVR/Reimpl/BaseRenderModels.h
	OpenOVR/Reimpl/BaseScreenshots.h
	OpenOVR/Reimpl/BaseSettings.h
	OpenOVR/Reimpl/BaseSystem.h
	OpenOVR/Reimpl/Interfaces.h
	OpenOVR/Reimpl/static_bases.gen.h
	OpenOVR/resources.h
	OpenOVR/stdafx.h
	OpenOVR/steamvr_abi.h
	OpenOVR/targetver.h

	# Newly-added classes
	OpenOVR/Drivers/DriverManager.cpp
	OpenOVR/Drivers/DriverManager.h

	# While not actually part of OCCore, list all the OpenVR interfaces here so they are easily accessable in IDE
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRClientCore_002.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRClientCore_003.h
	SplitOpenVRHeaders/OpenVR/custom_interfaces/IVRCompositor_017.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRApplications_004.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRApplications_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRApplications_006.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRChaperoneSetup_005.h
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
	SplitOpenVRHeaders/OpenVR/interfaces/IVRInput_004.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRIOBuffer_001.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVRNotifications_002.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_010.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_011.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_012.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_013.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_014.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_016.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_017.h
	SplitOpenVRHeaders/OpenVR/interfaces/IVROverlay_018.h
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
	SplitOpenVRHeaders/OpenVR/interfaces/openvr.h
	SplitOpenVRHeaders/OpenVR/interfaces/vrannotation.h
	SplitOpenVRHeaders/OpenVR/interfaces/vrtypes.h

	# OpenVR Driver headers
	SplitOpenVRHeaders/OpenVR/interfaces/driver_itrackeddevicedriverprovider.h
	SplitOpenVRHeaders/OpenVR/interfaces/driver_IVRServerDriverHost_005.h
	SplitOpenVRHeaders/OpenVR/interfaces/driver_ITrackedDeviceServerDriver_005.h
)
target_include_directories(OCCore PUBLIC OpenOVR) # TODO make this private and put the public headers elsewhere
target_include_directories(OCCore PRIVATE BundledLibs)
target_compile_definitions(OCCore PRIVATE -DSUPPORT_DX)
target_link_libraries(OCCore OpenVR ovr ovrkernel)

source_group(OpenVR REGULAR_EXPRESSION OpenOVR/OpenVR/interfaces/*)
source_group(OpenVR\\Drivers REGULAR_EXPRESSION OpenOVR/OpenVR/interfaces/driver_*)
source_group(OpenVR\\Custom REGULAR_EXPRESSION OpenOVR/OpenVR/custom_interfaces/*)
source_group(Interfaces REGULAR_EXPRESSION OpenOVR/Reimpl/Base*)
source_group(Interfaces\\Declarations REGULAR_EXPRESSION OpenOVR/Reimpl/CVR*)

# === OCOVR ===
add_library(OCOVR SHARED
	OCOVR/dllmain.cpp

	# Resources don't get nicely included in a library file
	${OVR_RESOURCES}
)
target_link_libraries(OCOVR OCCore)
