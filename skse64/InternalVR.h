#pragma once

#include "skse64/PluginAPI.h"

// These openvr headers have been modified to have a fixed namespace
// so that both headers may be included, this is for version retargeting
// This facilitates retargeting Skyrims IVRSystem_019 to the target version

#include "common/ICriticalSection.h"
#include <unordered_map>
#include <set>

#ifndef OPENVR_INTERFACE_INTERNAL
#define OPENVR_INTERFACE_INTERNAL
#endif
#include "openvr_1_0_12.h"
#ifdef INVALID_SHARED_TEXTURE_HANDLE
#undef INVALID_SHARED_TEXTURE_HANDLE
#endif
#ifdef INVALID_TRACKED_CAMERA_HANDLE
#undef INVALID_TRACKED_CAMERA_HANDLE
#endif
#include "openvr_1_7_15.h"
#ifdef INVALID_SHARED_TEXTURE_HANDLE
#undef INVALID_SHARED_TEXTURE_HANDLE
#endif
#ifdef INVALID_TRACKED_CAMERA_HANDLE
#undef INVALID_TRACKED_CAMERA_HANDLE
#endif

namespace vr_src = vr_1_0_12;
namespace vr_dst = vr_1_7_15;

STATIC_ASSERT(sizeof(vr_src::DistortionCoordinates_t) == sizeof(vr_dst::DistortionCoordinates_t));
STATIC_ASSERT(sizeof(vr_src::HmdMatrix34_t) == sizeof(vr_dst::HmdMatrix34_t));
STATIC_ASSERT(sizeof(vr_src::Texture_t) == sizeof(vr_dst::Texture_t));
STATIC_ASSERT(sizeof(vr_src::TrackedDevicePose_t) == sizeof(vr_dst::TrackedDevicePose_t));
STATIC_ASSERT(sizeof(vr_src::VRTextureBounds_t) == sizeof(vr_dst::VRTextureBounds_t));
//STATIC_ASSERT(sizeof(vr_src::Compositor_FrameTiming) == sizeof(vr_dst::Compositor_FrameTiming));
STATIC_ASSERT(sizeof(vr_src::Compositor_CumulativeStats) == sizeof(vr_dst::Compositor_CumulativeStats));
//STATIC_ASSERT(sizeof(vr_src::VREvent_t) == sizeof(vr_dst::VREvent_t));

class IVRSystemProxy : public vr_src::IVRSystem
{
public:
	IVRSystemProxy(vr_dst::IVRSystem* targetSystem) : m_system(targetSystem) { }

	virtual void GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight) override { m_system->GetRecommendedRenderTargetSize(pnWidth, pnHeight); }
	virtual vr_src::HmdMatrix44_t GetProjectionMatrix(vr_src::EVREye eEye, float fNearZ, float fFarZ) override { return *reinterpret_cast<vr_src::HmdMatrix44_t*>(&m_system->GetProjectionMatrix(static_cast<vr_dst::EVREye>(eEye), fNearZ, fFarZ)); };
	virtual void GetProjectionRaw(vr_src::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom) override { m_system->GetProjectionRaw(static_cast<vr_dst::EVREye>(eEye), pfLeft, pfRight, pfTop, pfBottom); };
	virtual bool ComputeDistortion(vr_src::EVREye eEye, float fU, float fV, vr_src::DistortionCoordinates_t *pDistortionCoordinates) override { return m_system->ComputeDistortion(static_cast<vr_dst::EVREye>(eEye), fU, fV, reinterpret_cast<vr_dst::DistortionCoordinates_t*>(pDistortionCoordinates)); };
	virtual vr_src::HmdMatrix34_t GetEyeToHeadTransform(vr_src::EVREye eEye) override { return *reinterpret_cast<vr_src::HmdMatrix34_t*>(&m_system->GetEyeToHeadTransform(static_cast<vr_dst::EVREye>(eEye))); };
	virtual bool GetTimeSinceLastVsync(float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter) override { return m_system->GetTimeSinceLastVsync(pfSecondsSinceLastVsync, pulFrameCounter); };
	virtual int32_t GetD3D9AdapterIndex() override { return m_system->GetD3D9AdapterIndex(); };
	virtual void GetDXGIOutputInfo(int32_t *pnAdapterIndex) override { m_system->GetDXGIOutputInfo(pnAdapterIndex); };
	virtual void GetOutputDevice(uint64_t *pnDevice, vr_src::ETextureType textureType, VkInstance_T *pInstance = nullptr) override { m_system->GetOutputDevice(pnDevice, static_cast<vr_dst::ETextureType>(textureType), pInstance); };
	virtual bool IsDisplayOnDesktop() override { return m_system->IsDisplayOnDesktop(); }
	virtual bool SetDisplayVisibility(bool bIsVisibleOnDesktop) override { return m_system->SetDisplayVisibility(bIsVisibleOnDesktop); }
	virtual void GetDeviceToAbsoluteTrackingPose(vr_src::ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow, VR_ARRAY_COUNT(unTrackedDevicePoseArrayCount) vr_src::TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) override { m_system->GetDeviceToAbsoluteTrackingPose(static_cast<vr_dst::ETrackingUniverseOrigin>(eOrigin), fPredictedSecondsToPhotonsFromNow, reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pTrackedDevicePoseArray), unTrackedDevicePoseArrayCount); }
	virtual void ResetSeatedZeroPose() override { m_system->ResetSeatedZeroPose(); }
	virtual vr_src::HmdMatrix34_t GetSeatedZeroPoseToStandingAbsoluteTrackingPose() override { return *reinterpret_cast<vr_src::HmdMatrix34_t*>(&m_system->GetSeatedZeroPoseToStandingAbsoluteTrackingPose()); }
	virtual vr_src::HmdMatrix34_t GetRawZeroPoseToStandingAbsoluteTrackingPose() override { return *reinterpret_cast<vr_src::HmdMatrix34_t*>(&m_system->GetRawZeroPoseToStandingAbsoluteTrackingPose()); }
	virtual uint32_t GetSortedTrackedDeviceIndicesOfClass(vr_src::ETrackedDeviceClass eTrackedDeviceClass, VR_ARRAY_COUNT(unTrackedDeviceIndexArrayCount) vr_src::TrackedDeviceIndex_t *punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount, vr_src::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex = vr_src::k_unTrackedDeviceIndex_Hmd) override { return m_system->GetSortedTrackedDeviceIndicesOfClass(static_cast<vr_dst::ETrackedDeviceClass>(eTrackedDeviceClass), punTrackedDeviceIndexArray, unTrackedDeviceIndexArrayCount, unRelativeToTrackedDeviceIndex); }
	virtual vr_src::EDeviceActivityLevel GetTrackedDeviceActivityLevel(vr_src::TrackedDeviceIndex_t unDeviceId) override { return static_cast<vr_src::EDeviceActivityLevel>(m_system->GetTrackedDeviceActivityLevel(unDeviceId)); }
	virtual void ApplyTransform(vr_src::TrackedDevicePose_t *pOutputPose, const vr_src::TrackedDevicePose_t *pTrackedDevicePose, const vr_src::HmdMatrix34_t *pTransform) override { m_system->ApplyTransform(reinterpret_cast<vr_dst::TrackedDevicePose_t *>(pOutputPose), reinterpret_cast<const vr_dst::TrackedDevicePose_t*>(pTrackedDevicePose), reinterpret_cast<const vr_dst::HmdMatrix34_t*>(pTransform)); }
	virtual vr_src::TrackedDeviceIndex_t GetTrackedDeviceIndexForControllerRole(vr_src::ETrackedControllerRole unDeviceType) override { return m_system->GetTrackedDeviceIndexForControllerRole(static_cast<vr_dst::ETrackedControllerRole>(unDeviceType)); }
	virtual vr_src::ETrackedControllerRole GetControllerRoleForTrackedDeviceIndex(vr_src::TrackedDeviceIndex_t unDeviceIndex) override { return static_cast<vr_src::ETrackedControllerRole>(m_system->GetControllerRoleForTrackedDeviceIndex(unDeviceIndex)); }
	virtual vr_src::ETrackedDeviceClass GetTrackedDeviceClass(vr_src::TrackedDeviceIndex_t unDeviceIndex) override { return static_cast<vr_src::ETrackedDeviceClass>(m_system->GetTrackedDeviceClass(unDeviceIndex)); }
	virtual bool IsTrackedDeviceConnected(vr_src::TrackedDeviceIndex_t unDeviceIndex) override { return m_system->IsTrackedDeviceConnected(unDeviceIndex); }
	virtual bool GetBoolTrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::ETrackedPropertyError *pError = 0L) override { return m_system->GetBoolTrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError)); }
	virtual float GetFloatTrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::ETrackedPropertyError *pError = 0L) override { return m_system->GetFloatTrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError)); }
	virtual int32_t GetInt32TrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::ETrackedPropertyError *pError = 0L) override { return m_system->GetInt32TrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError)); }
	virtual uint64_t GetUint64TrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::ETrackedPropertyError *pError = 0L) override { return m_system->GetUint64TrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError)); }
	virtual vr_src::HmdMatrix34_t GetMatrix34TrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::ETrackedPropertyError *pError = 0L) override { return *reinterpret_cast<vr_src::HmdMatrix34_t*>(&m_system->GetMatrix34TrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError))); }
	virtual uint32_t GetArrayTrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::PropertyTypeTag_t propType, void *pBuffer, uint32_t unBufferSize, vr_src::ETrackedPropertyError *pError = 0L) override { return m_system->GetArrayTrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), propType, pBuffer, unBufferSize, reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError)); }
	virtual uint32_t GetStringTrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, vr_src::ETrackedPropertyError *pError = 0L) override { return m_system->GetStringTrackedDeviceProperty(unDeviceIndex, static_cast<vr_dst::ETrackedDeviceProperty>(prop), pchValue, unBufferSize, reinterpret_cast<vr_dst::ETrackedPropertyError*>(pError)); }
	virtual const char *GetPropErrorNameFromEnum(vr_src::ETrackedPropertyError error) override { return m_system->GetPropErrorNameFromEnum(static_cast<vr_dst::ETrackedPropertyError>(error)); }
	virtual bool PollNextEvent(vr_src::VREvent_t *pEvent, uint32_t uncbVREvent) override { return m_system->PollNextEvent(reinterpret_cast<vr_dst::VREvent_t*>(pEvent), uncbVREvent); }
	virtual bool PollNextEventWithPose(vr_src::ETrackingUniverseOrigin eOrigin, vr_src::VREvent_t *pEvent, uint32_t uncbVREvent, vr_src::TrackedDevicePose_t *pTrackedDevicePose) override { return m_system->PollNextEventWithPose(static_cast<vr_dst::ETrackingUniverseOrigin>(eOrigin), reinterpret_cast<vr_dst::VREvent_t*>(pEvent), uncbVREvent, reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pTrackedDevicePose)); }
	virtual const char *GetEventTypeNameFromEnum(vr_src::EVREventType eType) override { return m_system->GetEventTypeNameFromEnum(static_cast<vr_dst::EVREventType>(eType)); }
	virtual vr_src::HiddenAreaMesh_t GetHiddenAreaMesh(vr_src::EVREye eEye, vr_src::EHiddenAreaMeshType type = vr_src::k_eHiddenAreaMesh_Standard) override { return *reinterpret_cast<vr_src::HiddenAreaMesh_t*>(&m_system->GetHiddenAreaMesh(static_cast<vr_dst::EVREye>(eEye), static_cast<vr_dst::EHiddenAreaMeshType>(type))); }
	virtual bool GetControllerState(vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, vr_src::VRControllerState_t *pControllerState, uint32_t unControllerStateSize) override { return m_system->GetControllerState(unControllerDeviceIndex, reinterpret_cast<vr_dst::VRControllerState_t*>(pControllerState), unControllerStateSize); }
	virtual bool GetControllerStateWithPose(vr_src::ETrackingUniverseOrigin eOrigin, vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, vr_src::VRControllerState_t *pControllerState, uint32_t unControllerStateSize, vr_src::TrackedDevicePose_t *pTrackedDevicePose) override { return m_system->GetControllerStateWithPose(static_cast<vr_dst::ETrackingUniverseOrigin>(eOrigin), unControllerDeviceIndex, reinterpret_cast<vr_dst::VRControllerState_t*>(pControllerState), unControllerStateSize, reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pTrackedDevicePose)); }
	virtual void TriggerHapticPulse(vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec) override { m_system->TriggerHapticPulse(unControllerDeviceIndex, unAxisId, usDurationMicroSec); }
	virtual const char *GetButtonIdNameFromEnum(vr_src::EVRButtonId eButtonId) override { return m_system->GetButtonIdNameFromEnum(static_cast<vr_dst::EVRButtonId>(eButtonId)); }
	virtual const char *GetControllerAxisTypeNameFromEnum(vr_src::EVRControllerAxisType eAxisType) override { return m_system->GetControllerAxisTypeNameFromEnum(static_cast<vr_dst::EVRControllerAxisType>(eAxisType)); }
	virtual bool IsInputAvailable() override { return m_system->IsInputAvailable(); }
	virtual bool IsSteamVRDrawingControllers() override { return m_system->IsSteamVRDrawingControllers(); }
	virtual bool ShouldApplicationPause() override { return m_system->ShouldApplicationPause(); }
	virtual bool ShouldApplicationReduceRenderingWork() override { return m_system->ShouldApplicationReduceRenderingWork(); }
	virtual uint32_t DriverDebugRequest(vr_src::TrackedDeviceIndex_t unDeviceIndex, const char *pchRequest, VR_OUT_STRING() char *pchResponseBuffer, uint32_t unResponseBufferSize) override { /*return m_system->DriverDebugRequest(unDeviceIndex, pchRequest, pchResponseBuffer, unResponseBufferSize);*/ return 0; }
	virtual vr_src::EVRFirmwareError PerformFirmwareUpdate(vr_src::TrackedDeviceIndex_t unDeviceIndex) override { return static_cast<vr_src::EVRFirmwareError>(m_system->PerformFirmwareUpdate(unDeviceIndex)); }
	virtual void AcknowledgeQuit_Exiting() override { m_system->AcknowledgeQuit_Exiting(); }
	virtual void AcknowledgeQuit_UserPrompt() override { /*m_system->AcknowledgeQuit_UserPrompt();*/ }

protected:
	vr_dst::IVRSystem*	m_system;
};

class IVRCompositorProxy : public vr_src::IVRCompositor
{
public:
	IVRCompositorProxy(vr_dst::IVRCompositor* targetCompositor) : m_compositor(targetCompositor) { }

	virtual void SetTrackingSpace(vr_src::ETrackingUniverseOrigin eOrigin) override { m_compositor->SetTrackingSpace(static_cast<vr_dst::ETrackingUniverseOrigin>(eOrigin)); }
	virtual vr_src::ETrackingUniverseOrigin GetTrackingSpace() override { return static_cast<vr_src::ETrackingUniverseOrigin>(m_compositor->GetTrackingSpace()); };
	virtual vr_src::EVRCompositorError WaitGetPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) vr_src::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount, VR_ARRAY_COUNT(unGamePoseArrayCount) vr_src::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->WaitGetPoses(reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pRenderPoseArray), unRenderPoseArrayCount, reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pGamePoseArray), unGamePoseArrayCount)); };
	virtual vr_src::EVRCompositorError GetLastPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) vr_src::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount, VR_ARRAY_COUNT(unGamePoseArrayCount) vr_src::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->GetLastPoses(reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pRenderPoseArray), unRenderPoseArrayCount, reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pGamePoseArray), unGamePoseArrayCount)); };
	virtual vr_src::EVRCompositorError GetLastPoseForTrackedDeviceIndex(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::TrackedDevicePose_t *pOutputPose, vr_src::TrackedDevicePose_t *pOutputGamePose) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->GetLastPoseForTrackedDeviceIndex(unDeviceIndex, reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pOutputPose), reinterpret_cast<vr_dst::TrackedDevicePose_t*>(pOutputGamePose))); }
	virtual vr_src::EVRCompositorError Submit(vr_src::EVREye eEye, const vr_src::Texture_t *pTexture, const vr_src::VRTextureBounds_t* pBounds = 0, vr_src::EVRSubmitFlags nSubmitFlags = vr_src::Submit_Default) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->Submit(static_cast<vr_dst::EVREye>(eEye), reinterpret_cast<const vr_dst::Texture_t *>(pTexture), reinterpret_cast<const vr_dst::VRTextureBounds_t*>(pBounds), static_cast<vr_dst::EVRSubmitFlags>(nSubmitFlags))); };
	virtual void ClearLastSubmittedFrame() override { m_compositor->ClearLastSubmittedFrame(); };
	virtual void PostPresentHandoff() override { m_compositor->PostPresentHandoff(); };
	virtual bool GetFrameTiming(vr_src::Compositor_FrameTiming *pTiming, uint32_t unFramesAgo = 0) override { return m_compositor->GetFrameTiming(reinterpret_cast<vr_dst::Compositor_FrameTiming *>(pTiming), unFramesAgo); };
	virtual uint32_t GetFrameTimings(vr_src::Compositor_FrameTiming *pTiming, uint32_t nFrames) override { return m_compositor->GetFrameTimings(reinterpret_cast<vr_dst::Compositor_FrameTiming *>(pTiming), nFrames); };
	virtual float GetFrameTimeRemaining() override { return m_compositor->GetFrameTimeRemaining(); };
	virtual void GetCumulativeStats(vr_src::Compositor_CumulativeStats *pStats, uint32_t nStatsSizeInBytes) override { m_compositor->GetCumulativeStats(reinterpret_cast<vr_dst::Compositor_CumulativeStats *>(pStats), nStatsSizeInBytes); };
	virtual void FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground = false) override { m_compositor->FadeToColor(fSeconds, fRed, fGreen, fBlue, fAlpha, bBackground); };
	virtual vr_src::HmdColor_t GetCurrentFadeColor(bool bBackground = false) override { return *reinterpret_cast<vr_src::HmdColor_t*>(&m_compositor->GetCurrentFadeColor(bBackground)); };
	virtual void FadeGrid(float fSeconds, bool bFadeIn) override { m_compositor->FadeGrid(fSeconds, bFadeIn); };
	virtual float GetCurrentGridAlpha() override { return m_compositor->GetCurrentGridAlpha(); }
	virtual vr_src::EVRCompositorError SetSkyboxOverride(VR_ARRAY_COUNT(unTextureCount) const vr_src::Texture_t *pTextures, uint32_t unTextureCount) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->SetSkyboxOverride(reinterpret_cast<const vr_dst::Texture_t *>(pTextures), unTextureCount)); };
	virtual void ClearSkyboxOverride() override { m_compositor->ClearSkyboxOverride(); };
	virtual void CompositorBringToFront() override { m_compositor->CompositorBringToFront(); };
	virtual void CompositorGoToBack() override { m_compositor->CompositorGoToBack(); };
	virtual void CompositorQuit() override { m_compositor->CompositorQuit(); };
	virtual bool IsFullscreen() override { return m_compositor->IsFullscreen(); }
	virtual uint32_t GetCurrentSceneFocusProcess() override { return m_compositor->GetCurrentSceneFocusProcess(); }
	virtual uint32_t GetLastFrameRenderer() override { return m_compositor->GetLastFrameRenderer(); }
	virtual bool CanRenderScene() override { return m_compositor->CanRenderScene(); }
	virtual void ShowMirrorWindow() override { m_compositor->ShowMirrorWindow(); };
	virtual void HideMirrorWindow() override { m_compositor->HideMirrorWindow(); };
	virtual bool IsMirrorWindowVisible() override { return m_compositor->IsMirrorWindowVisible(); }
	virtual void CompositorDumpImages() override { m_compositor->CompositorDumpImages(); };
	virtual bool ShouldAppRenderWithLowResources() override { return m_compositor->ShouldAppRenderWithLowResources(); }
	virtual void ForceInterleavedReprojectionOn(bool bOverride) override { m_compositor->ForceInterleavedReprojectionOn(bOverride); };
	virtual void ForceReconnectProcess() override { m_compositor->ForceReconnectProcess(); };
	virtual void SuspendRendering(bool bSuspend) override { m_compositor->SuspendRendering(bSuspend); };
	virtual vr_src::EVRCompositorError GetMirrorTextureD3D11(vr_src::EVREye eEye, void *pD3D11DeviceOrResource, void **ppD3D11ShaderResourceView) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->GetMirrorTextureD3D11(static_cast<vr_dst::EVREye>(eEye), pD3D11DeviceOrResource, ppD3D11ShaderResourceView)); };
	virtual void ReleaseMirrorTextureD3D11(void *pD3D11ShaderResourceView) override { m_compositor->ReleaseMirrorTextureD3D11(pD3D11ShaderResourceView); };
	virtual vr_src::EVRCompositorError GetMirrorTextureGL(vr_src::EVREye eEye, vr_src::glUInt_t *pglTextureId, vr_src::glSharedTextureHandle_t *pglSharedTextureHandle) override { return static_cast<vr_src::EVRCompositorError>(m_compositor->GetMirrorTextureGL(static_cast<vr_dst::EVREye>(eEye), pglTextureId, pglSharedTextureHandle)); };
	virtual bool ReleaseSharedGLTexture(vr_src::glUInt_t glTextureId, vr_src::glSharedTextureHandle_t glSharedTextureHandle) override { return m_compositor->ReleaseSharedGLTexture(glTextureId, glSharedTextureHandle); };
	virtual void LockGLSharedTextureForAccess(vr_src::glSharedTextureHandle_t glSharedTextureHandle) override { m_compositor->LockGLSharedTextureForAccess(glSharedTextureHandle); };
	virtual void UnlockGLSharedTextureForAccess(vr_src::glSharedTextureHandle_t glSharedTextureHandle) override { m_compositor->UnlockGLSharedTextureForAccess(glSharedTextureHandle); };
	virtual uint32_t GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char *pchValue, uint32_t unBufferSize) override { return m_compositor->GetVulkanInstanceExtensionsRequired(pchValue, unBufferSize); }
	virtual uint32_t GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T *pPhysicalDevice, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize) override { return m_compositor->GetVulkanDeviceExtensionsRequired(pPhysicalDevice, pchValue, unBufferSize); }
	virtual void SetExplicitTimingMode(vr_src::EVRCompositorTimingMode eTimingMode) override { m_compositor->SetExplicitTimingMode(static_cast<vr_dst::EVRCompositorTimingMode>(eTimingMode)); };
	virtual vr_src::EVRCompositorError SubmitExplicitTimingData() override { return static_cast<vr_src::EVRCompositorError>(m_compositor->SubmitExplicitTimingData()); }

protected:
	vr_dst::IVRCompositor*	m_compositor;
};

namespace vr_1_7_15
{
	namespace Exports
	{
		typedef void * (*VR_GetGenericInterface)(const char *pchInterfaceVersion, vr_src::EVRInitError *peError);
		typedef uint32_t(*VR_InitInternal2)(vr_src::EVRInitError *peError, vr_src::EVRApplicationType eApplicationType, const char *pStartupInfo);
		typedef void(*VR_ShutdownInternal)();
		typedef bool(*VR_IsHmdPresent)();
		typedef bool(*VR_IsRuntimeInstalled)();
		typedef bool(*VR_GetRuntimePath)(VR_OUT_STRING() char *pchPathBuffer, uint32_t unBufferSize, uint32_t *punRequiredBufferSize);
		typedef const char *(*VR_GetVRInitErrorAsSymbol)(vr_src::EVRInitError error);
		typedef const char *(*VR_GetVRInitErrorAsEnglishDescription)(vr_src::EVRInitError error);
		typedef bool(*VR_IsInterfaceVersionValid)(const char *pchInterfaceVersion);
		typedef uint32_t(*VR_GetInitToken)();
	}

	namespace IAT
	{
		static uintptr_t	VR_GetGenericInterface;
		static uintptr_t	VR_InitInternal2;
		static uintptr_t	VR_ShutdownInternal;
		static uintptr_t	VR_IsHmdPresent;
		static uintptr_t	VR_IsRuntimeInstalled;
		static uintptr_t	VR_GetRuntimePath;
		static uintptr_t	VR_GetVRInitErrorAsSymbol;
		static uintptr_t	VR_GetVRInitErrorAsEnglishDescription;
		static uintptr_t	VR_IsInterfaceVersionValid;
		static uintptr_t	VR_GetInitToken;
	}

	struct Exported
	{
		Exports::VR_GetGenericInterface					VR_GetGenericInterface;
		Exports::VR_InitInternal2						VR_InitInternal2;
		Exports::VR_ShutdownInternal					VR_ShutdownInternal;
		Exports::VR_IsHmdPresent						VR_IsHmdPresent;
		Exports::VR_IsRuntimeInstalled					VR_IsRuntimeInstalled;
		Exports::VR_GetRuntimePath						VR_GetRuntimePath;
		Exports::VR_GetVRInitErrorAsSymbol				VR_GetVRInitErrorAsSymbol;
		Exports::VR_GetVRInitErrorAsEnglishDescription	VR_GetVRInitErrorAsEnglishDescription;
		Exports::VR_IsInterfaceVersionValid				VR_IsInterfaceVersionValid;
		Exports::VR_GetInitToken						VR_GetInitToken;
	};

	inline uint32_t &VRToken()
	{
		static uint32_t token;
		return token;
	}

	class COpenVRContext
	{
	public:
		COpenVRContext(Exported& exports) : m_exports(exports) { Clear(); }

		inline void Clear()
		{
			m_pVRSystem = nullptr;
			m_pVRChaperone = nullptr;
			m_pVRChaperoneSetup = nullptr;
			m_pVRCompositor = nullptr;
			m_pVROverlay = nullptr;
			m_pVRRenderModels = nullptr;
			m_pVRExtendedDisplay = nullptr;
			m_pVRSettings = nullptr;
			m_pVRApplications = nullptr;
			m_pVRTrackedCamera = nullptr;
			m_pVRResources = nullptr;
			m_pVRScreenshots = nullptr;
			m_pVRDriverManager = nullptr;
			m_pVRInput = nullptr;
			m_pVRIOBuffer = nullptr;
			m_pVRSpatialAnchors = nullptr;
			m_pVRNotifications = nullptr;
			m_pVRDebug = nullptr;
		}

		inline void CheckClear()
		{
			if (VRToken() != m_exports.VR_GetInitToken())
			{
				Clear();
				VRToken() = m_exports.VR_GetInitToken();
			}
		}

		IVRSystem *VRSystem()
		{
			CheckClear();
			if (m_pVRSystem == nullptr)
			{
				EVRInitError eError;
				m_pVRSystem = (IVRSystem *)m_exports.VR_GetGenericInterface(IVRSystem_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRSystem;
		}
		IVRChaperone *VRChaperone()
		{
			CheckClear();
			if (m_pVRChaperone == nullptr)
			{
				EVRInitError eError;
				m_pVRChaperone = (IVRChaperone *)m_exports.VR_GetGenericInterface(IVRChaperone_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRChaperone;
		}

		IVRChaperoneSetup *VRChaperoneSetup()
		{
			CheckClear();
			if (m_pVRChaperoneSetup == nullptr)
			{
				EVRInitError eError;
				m_pVRChaperoneSetup = (IVRChaperoneSetup *)m_exports.VR_GetGenericInterface(IVRChaperoneSetup_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRChaperoneSetup;
		}

		IVRCompositor *VRCompositor()
		{
			CheckClear();
			if (m_pVRCompositor == nullptr)
			{
				EVRInitError eError;
				m_pVRCompositor = (IVRCompositor *)m_exports.VR_GetGenericInterface(IVRCompositor_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRCompositor;
		}

		IVROverlay *VROverlay()
		{
			CheckClear();
			if (m_pVROverlay == nullptr)
			{
				EVRInitError eError;
				m_pVROverlay = (IVROverlay *)m_exports.VR_GetGenericInterface(IVROverlay_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVROverlay;
		}

		IVRResources *VRResources()
		{
			CheckClear();
			if (m_pVRResources == nullptr)
			{
				EVRInitError eError;
				m_pVRResources = (IVRResources *)m_exports.VR_GetGenericInterface(IVRResources_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRResources;
		}

		IVRScreenshots *VRScreenshots()
		{
			CheckClear();
			if (m_pVRScreenshots == nullptr)
			{
				EVRInitError eError;
				m_pVRScreenshots = (IVRScreenshots *)m_exports.VR_GetGenericInterface(IVRScreenshots_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRScreenshots;
		}

		IVRRenderModels *VRRenderModels()
		{
			CheckClear();
			if (m_pVRRenderModels == nullptr)
			{
				EVRInitError eError;
				m_pVRRenderModels = (IVRRenderModels *)m_exports.VR_GetGenericInterface(IVRRenderModels_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRRenderModels;
		}

		IVRExtendedDisplay *VRExtendedDisplay()
		{
			CheckClear();
			if (m_pVRExtendedDisplay == nullptr)
			{
				EVRInitError eError;
				m_pVRExtendedDisplay = (IVRExtendedDisplay *)m_exports.VR_GetGenericInterface(IVRExtendedDisplay_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRExtendedDisplay;
		}

		IVRSettings *VRSettings()
		{
			CheckClear();
			if (m_pVRSettings == nullptr)
			{
				EVRInitError eError;
				m_pVRSettings = (IVRSettings *)m_exports.VR_GetGenericInterface(IVRSettings_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRSettings;
		}

		IVRApplications *VRApplications()
		{
			CheckClear();
			if (m_pVRApplications == nullptr)
			{
				EVRInitError eError;
				m_pVRApplications = (IVRApplications *)m_exports.VR_GetGenericInterface(IVRApplications_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRApplications;
		}

		IVRTrackedCamera *VRTrackedCamera()
		{
			CheckClear();
			if (m_pVRTrackedCamera == nullptr)
			{
				EVRInitError eError;
				m_pVRTrackedCamera = (IVRTrackedCamera *)m_exports.VR_GetGenericInterface(IVRTrackedCamera_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRTrackedCamera;
		}

		IVRDriverManager *VRDriverManager()
		{
			CheckClear();
			if (!m_pVRDriverManager)
			{
				EVRInitError eError;
				m_pVRDriverManager = (IVRDriverManager *)m_exports.VR_GetGenericInterface(IVRDriverManager_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRDriverManager;
		}

		IVRInput *VRInput()
		{
			CheckClear();
			if (!m_pVRInput)
			{
				EVRInitError eError;
				m_pVRInput = (IVRInput *)m_exports.VR_GetGenericInterface(IVRInput_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRInput;
		}

		IVRIOBuffer *VRIOBuffer()
		{
			if (!m_pVRIOBuffer)
			{
				EVRInitError eError;
				m_pVRIOBuffer = (IVRIOBuffer *)m_exports.VR_GetGenericInterface(IVRIOBuffer_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRIOBuffer;
		}

		IVRSpatialAnchors *VRSpatialAnchors()
		{
			CheckClear();
			if (!m_pVRSpatialAnchors)
			{
				EVRInitError eError;
				m_pVRSpatialAnchors = (IVRSpatialAnchors *)m_exports.VR_GetGenericInterface(IVRSpatialAnchors_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRSpatialAnchors;
		}

		IVRDebug *VRDebug()
		{
			CheckClear();
			if (!m_pVRDebug)
			{
				EVRInitError eError;
				m_pVRDebug = (IVRDebug *)m_exports.VR_GetGenericInterface(IVRDebug_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRDebug;
		}

		IVRNotifications *VRNotifications()
		{
			CheckClear();
			if (!m_pVRNotifications)
			{
				EVRInitError eError;
				m_pVRNotifications = (IVRNotifications *)m_exports.VR_GetGenericInterface(IVRNotifications_Version, reinterpret_cast<vr_src::EVRInitError*>(&eError));
			}
			return m_pVRNotifications;
		}

	private:
		Exported&			m_exports;
		IVRSystem			*m_pVRSystem;
		IVRChaperone		*m_pVRChaperone;
		IVRChaperoneSetup	*m_pVRChaperoneSetup;
		IVRCompositor		*m_pVRCompositor;
		IVROverlay			*m_pVROverlay;
		IVRResources		*m_pVRResources;
		IVRRenderModels		*m_pVRRenderModels;
		IVRExtendedDisplay	*m_pVRExtendedDisplay;
		IVRSettings			*m_pVRSettings;
		IVRApplications		*m_pVRApplications;
		IVRTrackedCamera	*m_pVRTrackedCamera;
		IVRScreenshots		*m_pVRScreenshots;
		IVRDriverManager	*m_pVRDriverManager;
		IVRInput			*m_pVRInput;
		IVRIOBuffer			*m_pVRIOBuffer;
		IVRSpatialAnchors   *m_pVRSpatialAnchors;
		IVRDebug			*m_pVRDebug;
		IVRNotifications	*m_pVRNotifications;
	};

	struct ActionHandles
	{
		bool						m_enabled;
		vr_dst::VRActionSetHandle_t m_defaultSetHandle;
		vr_dst::VRActionSetHandle_t m_legacySetHandle;

		struct Role
		{
			std::map<vr_dst::EVRButtonId, vr_dst::VRActionHandle_t> m_pressedMap;
			std::map<vr_dst::EVRButtonId, vr_dst::VRActionHandle_t> m_touchedMap;
			std::map<vr_dst::EVRButtonId, vr_dst::VRActionHandle_t> m_valueMap;
			std::map<vr_dst::EVRButtonId, vr_dst::VRActionHandle_t> m_value1Map;
			std::map<vr_dst::EVRButtonId, vr_dst::VRActionHandle_t> m_value2Map;
		};

		Role						m_leftRole;
		Role						m_rightRole;
		ICriticalSection			m_lock;
	};

	struct TrackedDevices
	{
		vr_dst::TrackedDeviceIndex_t									m_deviceIndex[k_unMaxTrackedDeviceCount];
		uint32_t														m_numDevices;
		std::unordered_map<vr_dst::TrackedDeviceIndex_t, std::string>	m_controllerTypeMap;
		ICriticalSection												m_lock;
	};
}

extern vr_dst::Exported vr_exports;
extern vr_dst::COpenVRContext vr_context;
extern vr_dst::ActionHandles vr_actionHandles;
extern vr_dst::TrackedDevices vr_devices;

template<typename T>
struct SortedPluginItem
{
	PluginHandle handle;
	int32_t sortOrder;
	T callback;

	bool operator<(const SortedPluginItem<T>& other) const
	{
		if (sortOrder == other.sortOrder)
		{
			return handle < other.handle;
		}

		return sortOrder < other.sortOrder;
	}
};

template<typename T>
struct SortedPluginCallbacks
{
	std::set<SortedPluginItem<T>>	m_callbacks;
	ICriticalSection				m_lock;
};

extern SortedPluginCallbacks<SKSEVRInterface::ControllerStateCallback>	g_controllerStateCallbacks;
extern SortedPluginCallbacks<SKSEVRInterface::WaitGetPosesCallback>		g_poseCallbacks;

namespace InternalVR
{
	void RegisterActionBindings();
	void UpdateTrackedDevices();
	void LoadActionHandles(bool reload = false);
	bool IsActionsEnabled();

	void RegisterForControllerState(PluginHandle plugin, int32_t sortOrder, SKSEVRInterface::ControllerStateCallback callback);
	void RegisterForPoses(PluginHandle plugin, int32_t sortOrder, SKSEVRInterface::WaitGetPosesCallback callback);

	void ExecuteControllerStateCallbacks(vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, vr_src::VRControllerState_t *pControllerState, uint32_t unControllerStateSize);
	void ExecutePoseCallbacks(vr_src::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount, vr_src::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);
}
