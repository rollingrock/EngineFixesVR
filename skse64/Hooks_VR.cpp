#include "Hooks_VR.h"
#include "skse64_common/SafeWrite.h"
#include "skse64_common/Utilities.h"
#include "GameVR.h"
#include "InternalVR.h"

namespace vr = vr_dst;

// ??_7BSOpenVR@@6B@
RelocAddr<uintptr_t> BSOpenVR_Vtable(0x017E6AA0);

static vr::Exports::VR_GetGenericInterface VR_GetGenericInterface_RealFunc;

static uint32_t packetNum = 0;

class IVRSystemHooked : protected IVRSystemProxy
{
public:
	explicit IVRSystemHooked(vr_dst::IVRSystem* targetSystem = nullptr) : IVRSystemProxy(targetSystem) { }

	virtual bool GetControllerState(vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, vr_src::VRControllerState_t *pControllerState, uint32_t unControllerStateSize) override
	{
#if 0
		// Testing bone input
		vr::EVRInputError err;

		uint32_t leftBoneCount = 0, rightBoneCount = 0;
		vr_context.VRInput()->GetBoneCount(vr_actionHandles.m_leftSkeletonHandle, &leftBoneCount);
		vr_context.VRInput()->GetBoneCount(vr_actionHandles.m_rightSkeletonHandle, &rightBoneCount);

		vr::VRBoneTransform_t* leftBones = new vr::VRBoneTransform_t[leftBoneCount];
		vr_context.VRInput()->GetSkeletalBoneData(vr_actionHandles.m_leftSkeletonHandle, vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController, leftBones, leftBoneCount);
		delete[] leftBones;

		vr::VRBoneTransform_t* rightBones = new vr::VRBoneTransform_t[rightBoneCount];
		vr_context.VRInput()->GetSkeletalBoneData(vr_actionHandles.m_rightSkeletonHandle, vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController, rightBones, rightBoneCount);
		delete[] rightBones;
#endif

		// We need to emulate controller state calls using Action bindings instead
		if (vr_actionHandles.m_enabled)
		{
			vr_src::TrackedDeviceIndex_t leftDevice = GetTrackedDeviceIndexForControllerRole(vr_src::ETrackedControllerRole::TrackedControllerRole_LeftHand);
			vr_src::TrackedDeviceIndex_t rightDevice = GetTrackedDeviceIndexForControllerRole(vr_src::ETrackedControllerRole::TrackedControllerRole_RightHand);

			vr::ActionHandles::Role* role = nullptr;
			if (unControllerDeviceIndex == leftDevice)
			{
				role = &vr_actionHandles.m_leftRole;
			}
			else if (unControllerDeviceIndex == rightDevice)
			{
				role = &vr_actionHandles.m_rightRole;
			}

			if (role)
			{
				// Zero the incoming state incase this didn't already happen
				memset(pControllerState, 0, unControllerStateSize);

				for (auto& button : role->m_pressedMap)
				{
					vr::InputDigitalActionData_t data;
					vr::EVRInputError err = vr_context.VRInput()->GetDigitalActionData(button.second, &data, sizeof(vr::InputDigitalActionData_t), vr::k_ulInvalidInputValueHandle);
					if (err == vr::EVRInputError::VRInputError_None) {
						if (data.bState && data.bActive) {
							pControllerState->ulButtonPressed |= (1ull << button.first);
						}
					}
				}
				for (auto& button : role->m_touchedMap)
				{
					vr::InputDigitalActionData_t data;
					vr::EVRInputError err = vr_context.VRInput()->GetDigitalActionData(button.second, &data, sizeof(vr::InputDigitalActionData_t), vr::k_ulInvalidInputValueHandle);
					if (err == vr::EVRInputError::VRInputError_None) {
						if (data.bState && data.bActive) {
							pControllerState->ulButtonTouched |= (1ull << button.first);
						}
					}
				}

				// Both X and Y
				for (auto& button : role->m_valueMap)
				{
					vr::InputAnalogActionData_t data;
					vr::EVRInputError err = vr_context.VRInput()->GetAnalogActionData(button.second, &data, sizeof(vr::InputAnalogActionData_t), vr::k_ulInvalidInputValueHandle);
					if (err == vr::EVRInputError::VRInputError_None) {
						if (data.bActive) {
							pControllerState->rAxis[button.first - vr::k_EButton_Axis0].x = data.x;
							pControllerState->rAxis[button.first - vr::k_EButton_Axis0].y = data.y;
						}
					}
				}

				// Only X
				for (auto& button : role->m_value1Map)
				{
					vr::InputAnalogActionData_t data;
					vr::EVRInputError err = vr_context.VRInput()->GetAnalogActionData(button.second, &data, sizeof(vr::InputAnalogActionData_t), vr::k_ulInvalidInputValueHandle);
					if (err == vr::EVRInputError::VRInputError_None) {
						if (data.bActive) {
							pControllerState->rAxis[button.first - vr::k_EButton_Axis0].x = data.x;
						}
					}
				}

				// Only Y
				for (auto& button : role->m_value2Map)
				{
					vr::InputAnalogActionData_t data;
					vr::EVRInputError err = vr_context.VRInput()->GetAnalogActionData(button.second, &data, sizeof(vr::InputAnalogActionData_t), vr::k_ulInvalidInputValueHandle);
					if (err == vr::EVRInputError::VRInputError_None) {
						if (data.bActive) {
							pControllerState->rAxis[button.first - vr::k_EButton_Axis0].y = data.y;
						}
					}
				}

				// Not sure this even does anything useful in Skyrim but we'll at least make it unique
				pControllerState->unPacketNum = ++packetNum;
				InternalVR::ExecuteControllerStateCallbacks(unControllerDeviceIndex, pControllerState, unControllerStateSize);
				return true;
			}
			else
			{
				return false;
			}
		}

		bool result = __super::GetControllerState(unControllerDeviceIndex, pControllerState, unControllerStateSize);
		if (result) {
			InternalVR::ExecuteControllerStateCallbacks(unControllerDeviceIndex, pControllerState, unControllerStateSize);
		}

		return result;
	}

	virtual bool GetControllerStateWithPose(vr_src::ETrackingUniverseOrigin eOrigin, vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, vr_src::VRControllerState_t *pControllerState, uint32_t unControllerStateSize, vr_src::TrackedDevicePose_t *pTrackedDevicePose) override
	{
		return __super::GetControllerStateWithPose(eOrigin, unControllerDeviceIndex, pControllerState, unControllerStateSize, pTrackedDevicePose);
	}

	virtual uint32_t GetStringTrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, vr_src::ETrackedPropertyError *pError) override
	{
		uint32_t ret = __super::GetStringTrackedDeviceProperty(unDeviceIndex, prop, pchValue, unBufferSize, pError);

		// Faking the TrackingSystemName is how simulate_controller_type works
		if (prop == vr_src::ETrackedDeviceProperty::Prop_TrackingSystemName_String && vr_actionHandles.m_enabled)
		{
			char buff[0x80];
			uint32_t typeSize = __super::GetStringTrackedDeviceProperty(unDeviceIndex, vr_src::Prop_ControllerType_String, buff, 0x80, pError);
			if (typeSize > 0 && _stricmp(buff, "knuckles") == 0) {
				strcpy_s(pchValue, unBufferSize, "oculus");
				return 6;
			}
		}

		return ret;
	}

	virtual int32_t GetInt32TrackedDeviceProperty(vr_src::TrackedDeviceIndex_t unDeviceIndex, vr_src::ETrackedDeviceProperty prop, vr_src::ETrackedPropertyError *pError) override
	{
		int32_t ret = __super::GetInt32TrackedDeviceProperty(unDeviceIndex, prop, pError);

		// Is this wrong? It reports Axis3 as None when it's actually a Joystick? Let's correct it for Skyrim so sticks work and Trackpad isn't clobbered
		if (prop == vr_src::ETrackedDeviceProperty::Prop_Axis3Type_Int32 && vr_actionHandles.m_enabled)
		{
			char buff[0x80];
			uint32_t typeSize = __super::GetStringTrackedDeviceProperty(unDeviceIndex, vr_src::Prop_ControllerType_String, buff, 0x80, pError);
			if (typeSize > 0 && _stricmp(buff, "knuckles") == 0) {
				return vr_src::k_eControllerAxis_Joystick;
			}
		}

		return ret;
	}

	// Since this is a proxy to a newer device, we need to query the new struct, then copy the old data
	virtual bool PollNextEvent(vr_src::VREvent_t *pEvent, uint32_t uncbVREvent) override
	{
		vr_dst::VREvent_t realEvent;
		bool result = m_system->PollNextEvent(&realEvent, sizeof(vr_dst::VREvent_t));
		if (result && vr_actionHandles.m_enabled)
		{
			switch (realEvent.eventType)
			{
			case vr_dst::VREvent_TrackedDeviceActivated:
			case vr_dst::VREvent_TrackedDeviceDeactivated:
			case vr_dst::VREvent_Input_TrackerActivated:
				InternalVR::UpdateTrackedDevices();
				break;
			case vr_dst::VREvent_Input_BindingLoadSuccessful:
			case vr_dst::VREvent_Input_ActionManifestReloaded:
			case vr_dst::VREvent_Input_BindingsUpdated:
				InternalVR::LoadActionHandles(true);
				break;
			default:
				break;
			}
		}
		memcpy(pEvent, &realEvent, uncbVREvent);
		return result;
	}

	void SetTargetSystem(vr_dst::IVRSystem* system)
	{
		m_system = system;
	}
};

class IVRCompositorHooked : protected IVRCompositorProxy
{
public:
	explicit IVRCompositorHooked(vr_dst::IVRCompositor* targetCompositor = nullptr) : IVRCompositorProxy(targetCompositor) { }

	virtual vr_src::EVRCompositorError WaitGetPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) vr_src::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount, VR_ARRAY_COUNT(unGamePoseArrayCount) vr_src::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount) override
	{
		auto result = __super::WaitGetPoses(pRenderPoseArray, unRenderPoseArrayCount, pGamePoseArray, unGamePoseArrayCount);

#if 0
		// Traverse all the connected trackers and acquire their poses since pose actions don't seem to work
		for (size_t i = 0; i < vr_devices.m_numDevices; ++i)
		{
			vr_src::TrackedDevicePose_t outputPose;
			vr_src::TrackedDevicePose_t gamePose;
			auto err = __super::GetLastPoseForTrackedDeviceIndex(vr_devices.m_deviceIndex[i], &outputPose, &gamePose);
			if (err == vr_src::VRCompositorError_None)
			{
				NiTransform xForm;
				HmdMatrixToNiTransform(xForm, gamePose.mDeviceToAbsoluteTracking);
				// TODO: Fire events that plugins can capture to acquire tracker data
				continue;
			}
		}
#endif
		InternalVR::ExecutePoseCallbacks(pRenderPoseArray, unRenderPoseArrayCount, pGamePoseArray, unGamePoseArrayCount);
		return result;
	};

	void SetTargetCompositor(vr_dst::IVRCompositor* compositor)
	{
		m_compositor = compositor;
	}
};

void * VR_CALLTYPE Hook_VR_GetGenericInterface_Execute(const char *pchInterfaceVersion, vr_src::EVRInitError *peError)
{
	// If version retarget queries fail, fallback to default interfaces
	if (_stricmp(pchInterfaceVersion, vr_src::IVRSystem_Version) == 0)
	{
		static IVRSystemHooked system;
		void* baseSystem = VR_GetGenericInterface_RealFunc(vr_dst::IVRSystem_Version, peError);
		if (baseSystem) {
			system.SetTargetSystem(static_cast<vr_dst::IVRSystem*>(baseSystem));
			return &system;
		}
	}
	else if (_stricmp(pchInterfaceVersion, vr_src::IVRCompositor_Version) == 0)
	{
		static IVRCompositorHooked system;
		void* baseCompositor = VR_GetGenericInterface_RealFunc(vr_dst::IVRCompositor_Version, peError);
		if (baseCompositor) {
			system.SetTargetCompositor(static_cast<vr_dst::IVRCompositor*>(baseCompositor));
			return &system;
		}
	}
	return VR_GetGenericInterface_RealFunc(pchInterfaceVersion, peError);
}

void PollNextEvent_Hooked(BSOpenVR* thisPtr)
{
	if (vr_actionHandles.m_enabled)
	{
		vr::VRActiveActionSet_t actionSet;
		actionSet.ulActionSet = vr_actionHandles.m_legacySetHandle;
		actionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
		actionSet.ulSecondaryActionSet = vr::k_ulInvalidInputValueHandle;
		actionSet.nPriority = 10;
		vr::EVRInputError err = vr_context.VRInput()->UpdateActionState(&actionSet, sizeof(vr::VRActiveActionSet_t), 1);

		// TODO: Maybe expose this to plugins?
	}

	thisPtr->PollNextEvent_Internal();
}

void Hooks_VR_Init(void)
{
	vr::IAT::VR_GetGenericInterface = (uintptr_t)GetIATAddr((UInt8 *)GetModuleHandle(NULL), "openvr_api.dll", "VR_GetGenericInterface");
	vr::IAT::VR_InitInternal2 = (uintptr_t)GetIATAddr((UInt8 *)GetModuleHandle(NULL), "openvr_api.dll", "VR_InitInternal2");
	vr::IAT::VR_ShutdownInternal = (uintptr_t)GetIATAddr((UInt8 *)GetModuleHandle(NULL), "openvr_api.dll", "VR_ShutdownInternal");
	vr::IAT::VR_GetInitToken = (uintptr_t)GetIATAddr((UInt8 *)GetModuleHandle(NULL), "openvr_api.dll", "VR_GetInitToken");

	// This occurs in Init on purpose so plugins can wrap this interface, SKSE should be first
	// so that it can emulate GetControllerState and pass the data along to existing plugins wrapping this interface

	VR_GetGenericInterface_RealFunc = (vr::Exports::VR_GetGenericInterface)*(uintptr_t *)vr::IAT::VR_GetGenericInterface;
	SafeWrite64(vr::IAT::VR_GetGenericInterface, (uintptr_t)Hook_VR_GetGenericInterface_Execute);

	vr_exports.VR_GetGenericInterface = (vr::Exports::VR_GetGenericInterface)*(uintptr_t *)vr::IAT::VR_GetGenericInterface;
	vr_exports.VR_InitInternal2 = (vr::Exports::VR_InitInternal2)*(uintptr_t *)vr::IAT::VR_InitInternal2;
	vr_exports.VR_ShutdownInternal = (vr::Exports::VR_ShutdownInternal)*(uintptr_t *)vr::IAT::VR_ShutdownInternal;
	vr_exports.VR_GetInitToken = (vr::Exports::VR_GetInitToken)*(uintptr_t *)vr::IAT::VR_GetInitToken;
}

void Hooks_VR_Commit(void)
{
	UInt32 vrInterfaceHooked = 0;
	if (GetConfigOption_UInt32("Input", "bActionBindings", &vrInterfaceHooked) && vrInterfaceHooked)
	{
		SafeWrite64(BSOpenVR_Vtable.GetUIntPtr() + 0x58, (uintptr_t)PollNextEvent_Hooked);
	}
}
