#include "InternalVR.h"
#include "skse64_common/Utilities.h"

#include <stdio.h>
#include <WinBase.h>

namespace vr = vr_dst;

vr::Exported vr_exports;
vr::COpenVRContext vr_context(vr_exports);
vr::ActionHandles vr_actionHandles;
vr::TrackedDevices vr_devices;

SortedPluginCallbacks<SKSEVRInterface::ControllerStateCallback>	g_controllerStateCallbacks;
SortedPluginCallbacks<SKSEVRInterface::WaitGetPosesCallback>	g_poseCallbacks;

bool InternalVR::IsActionsEnabled()
{
	return vr_actionHandles.m_enabled;
}

void InternalVR::RegisterActionBindings()
{
	UInt32	actionBindings = 0;
	vr_actionHandles.m_enabled = false;
	if (GetConfigOption_UInt32("Input", "bActionBindings", &actionBindings) && actionBindings)
	{
		char buff[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buff);
		strcat_s(buff, "\\bindings\\actions.json");
		vr::IVRInput* vrInput = vr_context.VRInput();
		if (vrInput) {
			vr::EVRInputError error = vrInput->SetActionManifestPath(buff);

			if (error != vr::EVRInputError::VRInputError_None)
				_MESSAGE("Error in registering action manifest to SteamVR Input (%d)", error);
			else
				_DMESSAGE("Loaded action manifest at %s", buff);

			LoadActionHandles();
			UpdateTrackedDevices();

			vr_actionHandles.m_enabled = true;
		}
	}
}

void InternalVR::UpdateTrackedDevices()
{
	IScopedCriticalSection locker(&vr_devices.m_lock);

	vr_devices.m_controllerTypeMap.clear();

	for (size_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
	{
		vr_devices.m_deviceIndex[i] = vr::k_unTrackedDeviceIndexInvalid;
	}

	uint32_t size = vr_context.VRSystem()->GetSortedTrackedDeviceIndicesOfClass(vr::TrackedDeviceClass_GenericTracker, nullptr, 0);
	if (size > 0)
	{
		vr_devices.m_numDevices = vr_context.VRSystem()->GetSortedTrackedDeviceIndicesOfClass(vr::TrackedDeviceClass_GenericTracker, vr_devices.m_deviceIndex, size);

		char buff[0x80];
		for (size_t i = 0; i < vr_devices.m_numDevices; ++i)
		{
			vr::ETrackedPropertyError propErr;
			uint32_t typeSize = vr_context.VRSystem()->GetStringTrackedDeviceProperty(vr_devices.m_deviceIndex[i], vr::Prop_ControllerType_String, buff, 0x80, &propErr);
			if (typeSize > 0) {
				vr_devices.m_controllerTypeMap.emplace(vr_devices.m_deviceIndex[i], buff);
			}
		}
	}
}

void InternalVR::LoadActionHandles(bool reload)
{
	IScopedCriticalSection locker(&vr_actionHandles.m_lock);

	auto vrInput = vr_context.VRInput();
	if (vrInput)
	{
		vr::EVRInputError error = vrInput->GetActionSetHandle("/actions/legacy", &vr_actionHandles.m_legacySetHandle);
		if (error != vr::EVRInputError::VRInputError_None)
		{
			_MESSAGE("Error loading legacy action set (%d)", error);
		}
		else if(!reload)
		{
			_DMESSAGE("Loading legacy action set %s", "/actions/legacy");
		}

		struct ActionNameHandle
		{
			const char* name;
			vr::VRActionHandle_t* handle;
		};

		ActionNameHandle actionList[] = {
			{ "/actions/legacy/in/Left_System_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_System] },
			{ "/actions/legacy/in/Left_System_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_System] },

			{ "/actions/legacy/in/Left_ApplicationMenu_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_ApplicationMenu] },
			{ "/actions/legacy/in/Left_ApplicationMenu_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_ApplicationMenu] },

			{ "/actions/legacy/in/Left_Grip_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_Grip] },
			{ "/actions/legacy/in/Left_Grip_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_Grip] },

			{ "/actions/legacy/in/Left_A_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_A] },
			{ "/actions/legacy/in/Left_A_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_A] },

			{ "/actions/legacy/in/Left_Axis0_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_Axis0] },
			{ "/actions/legacy/in/Left_Axis0_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_Axis0] },
			{ "/actions/legacy/in/Left_Axis0_Value", &vr_actionHandles.m_leftRole.m_valueMap[vr::k_EButton_Axis0] },

			{ "/actions/legacy/in/Left_Axis1_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_Axis1] },
			{ "/actions/legacy/in/Left_Axis1_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_Axis1] },
			{ "/actions/legacy/in/Left_Axis1_Value", &vr_actionHandles.m_leftRole.m_valueMap[vr::k_EButton_Axis1] },

			{ "/actions/legacy/in/Left_Axis2_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_Axis2] },
			{ "/actions/legacy/in/Left_Axis2_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_Axis2] },
			{ "/actions/legacy/in/Left_Axis2_Value1", &vr_actionHandles.m_leftRole.m_value1Map[vr::k_EButton_Axis2] },
			{ "/actions/legacy/in/Left_Axis2_Value2", &vr_actionHandles.m_leftRole.m_valueMap[vr::k_EButton_Axis2] },

			{ "/actions/legacy/in/Left_Axis3_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_Axis3] },
			{ "/actions/legacy/in/Left_Axis3_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_Axis3] },
			{ "/actions/legacy/in/Left_Axis3_Value", &vr_actionHandles.m_leftRole.m_valueMap[vr::k_EButton_Axis3] },

			{ "/actions/legacy/in/Left_Axis4_Press", &vr_actionHandles.m_leftRole.m_pressedMap[vr::k_EButton_Axis4] },
			{ "/actions/legacy/in/Left_Axis4_Touch", &vr_actionHandles.m_leftRole.m_touchedMap[vr::k_EButton_Axis4] },
			{ "/actions/legacy/in/Left_Axis4_Value_e0", &vr_actionHandles.m_leftRole.m_value1Map[vr::k_EButton_Axis4] },
			{ "/actions/legacy/in/Left_Axis4_Value_e1", &vr_actionHandles.m_leftRole.m_value2Map[vr::k_EButton_Axis4] },

			{ "/actions/legacy/in/Right_System_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_System] },
			{ "/actions/legacy/in/Right_System_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_System] },

			{ "/actions/legacy/in/Right_ApplicationMenu_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_ApplicationMenu] },
			{ "/actions/legacy/in/Right_ApplicationMenu_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_ApplicationMenu] },

			{ "/actions/legacy/in/Right_Grip_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_Grip] },
			{ "/actions/legacy/in/Right_Grip_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_Grip] },

			{ "/actions/legacy/in/Right_A_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_A] },
			{ "/actions/legacy/in/Right_A_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_A] },

			{ "/actions/legacy/in/Right_Axis0_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_Axis0] },
			{ "/actions/legacy/in/Right_Axis0_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_Axis0] },
			{ "/actions/legacy/in/Right_Axis0_Value", &vr_actionHandles.m_rightRole.m_valueMap[vr::k_EButton_Axis0] },

			{ "/actions/legacy/in/Right_Axis1_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_Axis1] },
			{ "/actions/legacy/in/Right_Axis1_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_Axis1] },
			{ "/actions/legacy/in/Right_Axis1_Value", &vr_actionHandles.m_rightRole.m_valueMap[vr::k_EButton_Axis1] },

			{ "/actions/legacy/in/Right_Axis2_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_Axis2] },
			{ "/actions/legacy/in/Right_Axis2_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_Axis2] },
			{ "/actions/legacy/in/Right_Axis2_Value1", &vr_actionHandles.m_rightRole.m_value1Map[vr::k_EButton_Axis2] },
			{ "/actions/legacy/in/Right_Axis2_Value2", &vr_actionHandles.m_rightRole.m_valueMap[vr::k_EButton_Axis2] },

			{ "/actions/legacy/in/Right_Axis3_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_Axis3] },
			{ "/actions/legacy/in/Right_Axis3_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_Axis3] },
			{ "/actions/legacy/in/Right_Axis3_Value", &vr_actionHandles.m_rightRole.m_valueMap[vr::k_EButton_Axis3] },

			{ "/actions/legacy/in/Right_Axis4_Press", &vr_actionHandles.m_rightRole.m_pressedMap[vr::k_EButton_Axis4] },
			{ "/actions/legacy/in/Right_Axis4_Touch", &vr_actionHandles.m_rightRole.m_touchedMap[vr::k_EButton_Axis4] },
			{ "/actions/legacy/in/Right_Axis4_Value_e0", &vr_actionHandles.m_rightRole.m_value1Map[vr::k_EButton_Axis4] },
			{ "/actions/legacy/in/Right_Axis4_Value_e1", &vr_actionHandles.m_rightRole.m_value2Map[vr::k_EButton_Axis4] },
		};

		for (size_t i = 0; i < sizeof(actionList) / sizeof(ActionNameHandle); ++i)
		{
			error = vrInput->GetActionHandle(actionList[i].name, actionList[i].handle);
			if (error != vr::EVRInputError::VRInputError_None)
			{
				_MESSAGE("Error loading action %s (%d)", actionList[i].name, error);
				continue;
			}
			else if(!reload)
			{
				_DMESSAGE("Loaded action %s at handle (%016llX)", actionList[i].name, *actionList[i].handle);
			}

#if 0
			// Exports the input binding paths about the action, this call can be used for button indicators
			uint32_t numBindings = 0;
			error = vrInput->GetActionBindingInfo(*actionList[i].handle, nullptr, sizeof(vr::InputBindingInfo_t), 0, &numBindings);
			if (error != vr::EVRInputError::VRInputError_None)
			{
				uint32_t returnedBindings = 0;
				vr::InputBindingInfo_t* bindings = new vr::InputBindingInfo_t[numBindings];
				error = vrInput->GetActionBindingInfo(*actionList[i].handle, bindings, sizeof(vr::InputBindingInfo_t), numBindings, &returnedBindings);
				gLog.Indent();
				for (uint32_t j = 0; j < numBindings; ++j)
				{
					_DMESSAGE("Binding %d info: Device Path: (%s) Input Path: (%s) Mode Name: (%s) Slot Name: (%s)", j, bindings[j].rchDevicePathName, bindings[j].rchInputPathName, bindings[j].rchModeName, bindings[j].rchSlotName);
				}
				gLog.Outdent();
				delete[] bindings;
			}
#endif
		}
	}
}

void InternalVR::RegisterForControllerState(PluginHandle plugin, int32_t sortOrder, SKSEVRInterface::ControllerStateCallback callback)
{
	IScopedCriticalSection locker(&g_controllerStateCallbacks.m_lock);
	_MESSAGE("Plugin %d registered for controller state with sort %d", plugin, sortOrder);
	g_controllerStateCallbacks.m_callbacks.insert({ plugin, sortOrder, callback });
}

void InternalVR::RegisterForPoses(PluginHandle plugin, int32_t sortOrder, SKSEVRInterface::WaitGetPosesCallback callback)
{
	IScopedCriticalSection locker(&g_poseCallbacks.m_lock);
	_MESSAGE("Plugin %d registered for poses with sort %d", plugin, sortOrder);
	g_poseCallbacks.m_callbacks.insert({ plugin, sortOrder, callback });
}

void InternalVR::ExecuteControllerStateCallbacks(vr_src::TrackedDeviceIndex_t unControllerDeviceIndex, vr_src::VRControllerState_t* pControllerState, uint32_t unControllerStateSize)
{
	IScopedCriticalSection locker(&g_controllerStateCallbacks.m_lock);

	bool state = true;
	if (vr_context.VRSystem()->IsTrackedDeviceConnected(unControllerDeviceIndex))
	{
		for (const auto& item : g_controllerStateCallbacks.m_callbacks)
		{
			item.callback(unControllerDeviceIndex, pControllerState, unControllerStateSize, state);
		}
	}
}

void InternalVR::ExecutePoseCallbacks(vr_src::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount, vr_src::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount)
{
	IScopedCriticalSection locker(&g_poseCallbacks.m_lock);
	for (const auto& item : g_poseCallbacks.m_callbacks)
	{
		item.callback(pRenderPoseArray, unRenderPoseArrayCount, pGamePoseArray, unGamePoseArrayCount);
	}
}
