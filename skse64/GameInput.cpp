#include "GameInput.h"
#include "GameVR.h"

bool InputEventDispatcher::IsGamepadEnabled(void)
{
	return !(*g_isUsingMotionControllers);
}

InputEventDispatcher* InputEventDispatcher::GetSingleton()
{
	// 5202C5E930BB4CD8F477F91C9434AB37DBDB10B3+7A
	static RelocPtr<InputEventDispatcher*> g_inputEventDispatcher(0x02FEA520);
	return *g_inputEventDispatcher;
}

InputManager * InputManager::GetSingleton(void)
{
	// 61FAE6E8975F0FA7B3DD4D5A410A240E86A58F7B+E
	static RelocPtr<InputManager*> g_inputManager(0x02F8AAA0);
	return *g_inputManager;
}

UInt8 InputManager::AllowTextInput(bool allow)
{
	if(allow)
	{
		if(allowTextInput == 0xFF)
			_WARNING("InputManager::AllowTextInput: counter overflow");
		else
			allowTextInput++;
	}
	else
	{
		if(allowTextInput == 0)
			_WARNING("InputManager::AllowTextInput: counter underflow");
		else
			allowTextInput--;
	}

	if(IsConsoleMode())
		Console_Print("%s text input, count = %d", allow ? "allowed" : "disallowed", allowTextInput);

	return allowTextInput;
}

UInt32 InputManager::GetDeviceType()
{
	if (*g_isUsingMotionControllers)
	{
		return GetDeviceTypeFromVR(*g_leftHandedMode);
	}
	else
	{
		return kDeviceType_Gamepad;
	}
}

UInt32 InputManager::GetDeviceTypeFromVR(bool primary)
{
	switch ((*g_openVR)->GetControllerType())
	{
	case BSOpenVR::kControllerType_Oculus:
		return primary ? kDeviceType_OculusPrimary : kDeviceType_OculusSecondary;
		break;
	case BSOpenVR::kControllerType_WindowsMR:
		return primary ? kDeviceType_OculusPrimary : kDeviceType_OculusSecondary;
		break;
	default:
		break;
	}

	// When the controller type isnt recognized, treat as Vive
	return primary ? kDeviceType_VivePrimary : kDeviceType_ViveSecondary;
}

UInt32 InputManager::GetDeviceOffsetForDevice(UInt32 deviceType)
{
	switch (deviceType) {
	case kDeviceType_VivePrimary:
		return InputMap::kMacro_VivePrimaryOffset;
		break;
	case kDeviceType_ViveSecondary:
		return InputMap::kMacro_ViveSecondaryOffset;
		break;
	case kDeviceType_OculusPrimary:
		return InputMap::kMacro_OculusPrimaryOffset;
		break;
	case kDeviceType_OculusSecondary:
		return InputMap::kMacro_OculusSecondaryOffset;
		break;
	case kDeviceType_WindowsMRPrimary:
		return InputMap::kMacro_WindowsMRPrimaryOffset;
		break;
	case kDeviceType_WindowsMRSecondary:
		return InputMap::kMacro_WindowsMRSecondaryOffset;
		break;
	case kDeviceType_Gamepad:
		return InputMap::kMacro_GamepadOffset;
		break;
	case kDeviceType_Mouse:
		return InputMap::kMacro_MouseButtonOffset;
		break;
	case kDeviceType_Keyboard:
		return InputMap::kMacro_KeyboardOffset;
		break;
	default:
		return InputMap::kMacro_VivePrimaryOffset;
		break;
	}
}

UInt32 InputManager::GetDeviceTypeFromKeyCode(UInt32 keyCode)
{
	UInt32 deviceType = kDeviceType_VivePrimary;
	if (keyCode >= InputMap::kMacro_WindowsMRSecondaryOffset) {
		deviceType = kDeviceType_WindowsMRSecondary;
	}
	else if (keyCode >= InputMap::kMacro_WindowsMRPrimaryOffset) {
		deviceType = kDeviceType_WindowsMRPrimary;
	}
	else if (keyCode >= InputMap::kMacro_OculusSecondaryOffset) {
		deviceType = kDeviceType_OculusSecondary;
	}
	else if (keyCode >= InputMap::kMacro_OculusPrimaryOffset) {
		deviceType = kDeviceType_OculusPrimary;
	}
	else if (keyCode >= InputMap::kMacro_ViveSecondaryOffset) {
		deviceType = kDeviceType_ViveSecondary;
	}
	else if (keyCode >= InputMap::kMacro_VivePrimaryOffset) {
		deviceType = kDeviceType_VivePrimary;
	}
	else if (keyCode >= InputMap::kMacro_GamepadOffset) {
		deviceType = kDeviceType_Gamepad;
	}
	else if (keyCode >= InputMap::kMacro_MouseWheelOffset || keyCode >= InputMap::kMacro_MouseButtonOffset) {
		deviceType = kDeviceType_Mouse;
	}
	else if (keyCode >= InputMap::kMacro_KeyboardOffset) {
		deviceType = kDeviceType_Keyboard;
	}
	return deviceType;
}

UInt32 InputManager::GetMappedKey(BSFixedString name, UInt32 deviceType, UInt32 contextIdx)
{
	ASSERT(contextIdx < kContextCount);

	tArray<InputContext::Mapping> * mappings;
	switch (deviceType)
	{
	case kDeviceType_Mouse:
	case kDeviceType_Gamepad:
	case kDeviceType_VivePrimary:
	case kDeviceType_ViveSecondary:
	case kDeviceType_OculusPrimary:
	case kDeviceType_OculusSecondary:
	case kDeviceType_WindowsMRPrimary:
	case kDeviceType_WindowsMRSecondary:
	case kDeviceType_VirtualKeyboard:
		mappings = &context[contextIdx]->deviceMap[deviceType];
		break;
	default:
		mappings = &context[contextIdx]->deviceMap[kDeviceType_Keyboard];
		break;
	}

	for (UInt32 i = 0; i < mappings->count; i++)
	{
		InputContext::Mapping m;
		if (!mappings->GetNthItem(i, m))
			break;
		if (m.name == name)
			return m.buttonID;
	}

	// Unbound
	return 0xFF;
}

BSFixedString InputManager::GetMappedControl(UInt32 buttonID, UInt32 deviceType, UInt32 contextIdx)
{
	ASSERT(contextIdx < kContextCount);

	// 0xFF == unbound
	if (buttonID == 0xFF)
		return BSFixedString();

	tArray<InputContext::Mapping> * mappings;

	if (*g_isUsingMotionControllers)
	{
		// Look for the mapping on the primary hand
		UInt32 deviceList[] = {
			GetDeviceTypeFromVR(!(*g_leftHandedMode)),
			GetDeviceTypeFromVR(*g_leftHandedMode),
			kDeviceType_Mouse,
			kDeviceType_Keyboard
		};

		for (int d = 0; d < sizeof(deviceList) / sizeof(UInt32); ++d)
		{
			mappings = &context[contextIdx]->deviceMap[d];
			for (UInt32 i = 0; i < mappings->count; i++)
			{
				InputContext::Mapping m;
				if (!mappings->GetNthItem(i, m))
					break;
				if (m.buttonID == buttonID)
					return m.name;
			}
		}
	}
	else
	{
		switch (deviceType)
		{
		case kDeviceType_Mouse:
		case kDeviceType_Gamepad:
		case kDeviceType_VirtualKeyboard:
			mappings = &context[contextIdx]->deviceMap[deviceType];
			break;
		default:
			mappings = &context[contextIdx]->deviceMap[kDeviceType_Keyboard];
			break;
		}

		for (UInt32 i = 0; i < mappings->count; i++)
		{
			InputContext::Mapping m;
			if (!mappings->GetNthItem(i, m))
				break;
			if (m.buttonID == buttonID)
				return m.name;
		}
	}

	

	return BSFixedString();
}

PlayerControls * PlayerControls::GetSingleton(void)
{
	// F1E82AFF2615653A5A14A2E7C229B4B0466688EF+19
	static RelocPtr<PlayerControls*> g_playerControls(0x02F8AAA8);
	return *g_playerControls;
}

MenuControls * MenuControls::GetSingleton(void)
{
	// DC378767BEB0312EBDE098BC7E0CE53FCC296377+D9
	static RelocPtr<MenuControls*> g_menuControls(0x02FC52E8);
	return *g_menuControls;
}

 RelocPtr<bool> g_isUsingMotionControllers(0x01E717A8);
 RelocPtr<bool> g_leftHandedMode(0x01E71778);

#include "skse64/GameRTTI.h"

 void InputEvent::LogEvent()
 {
	switch (eventType)
	{
	case InputEvent::kEventType_MouseMove:
	{
		MouseMoveEvent * t = DYNAMIC_CAST(this, InputEvent, MouseMoveEvent);
		break;
	}
	case InputEvent::kEventType_Thumbstick:
	{
		ThumbstickEvent * t = DYNAMIC_CAST(this, InputEvent, ThumbstickEvent);
		_MESSAGE("Stick Control: %s X: %f Y: %f", t->GetControlID() ? t->GetControlID()->c_str() : "", t->x, t->y);
		break;
	}
	case InputEvent::kEventType_DeviceConnect:
	{
		break;
	}
	case InputEvent::kEventType_Button:
	{
		ButtonEvent * t = DYNAMIC_CAST(this, InputEvent, ButtonEvent);
		_MESSAGE("Button Control: %s State: %f Timer: %f Mask: %08X Flags: %08X", t->GetControlID() ? t->GetControlID()->c_str() : "", t->isDown, t->timer, t->keyMask, t->flags);
		break;
	}
	case InputEvent::kEventType_VrWandTouchpadPositionEvent:
	{
		VrWandTouchpadPositionEvent * t = DYNAMIC_CAST(this, InputEvent, VrWandTouchpadPositionEvent);
		_MESSAGE("Touchpad Pos Control: %s X: %f Y: %f", t->GetControlID() ? t->GetControlID()->c_str() : "", t->x, t->y);
		break;
	}
	case InputEvent::kEventType_VrWandTouchpadSwipeEvent:
	{
		VrWandTouchpadSwipeEvent * t = DYNAMIC_CAST(this, InputEvent, VrWandTouchpadSwipeEvent);
		_MESSAGE("Touchpad Swipe Control: %s X: %f Y: %f", t->GetControlID() ? t->GetControlID()->c_str() : "", t->x, t->y);
		break;
	}
	}
 }
