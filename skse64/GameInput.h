#pragma once

#include "GameTypes.h"
#include "GameEvents.h"
#include "GameUtilities.h"
#include "InputMap.h"
#include "NiTypes.h"

#include "openvr_1_0_12.h"

class PlayerControls;
class BGSEncounterZone;
class MenuEventHandler;
class ThumbstickEvent;
struct MenuOpenCloseEvent;
struct MenuModeChangeEvent;
struct TESFurnitureEvent;

// Note: These are different from those in Hooks_DI
enum
{
	kDeviceType_Keyboard = 0,
	kDeviceType_Mouse,
	kDeviceType_Gamepad,
	kDeviceType_VivePrimary,
	kDeviceType_ViveSecondary,
	kDeviceType_OculusPrimary,
	kDeviceType_OculusSecondary,
	kDeviceType_WindowsMRPrimary,
	kDeviceType_WindowsMRSecondary,
	kDeviceType_VirtualKeyboard,
	kDeviceType_Num
};

class BSIInputDevice
{
public:
	virtual ~BSIInputDevice();

	// add
	virtual void	Initialize() = 0;
	virtual	void	Process(float a_arg1) = 0;
	virtual	void	Unk_03(void) = 0;
	virtual	bool	GetKeyMapping(UInt32 a_key, BSFixedString& a_mapping) = 0;
	virtual UInt32	GetMappingKey(BSFixedString a_mapping) = 0;
	virtual void	Unk_06(void) = 0;
	virtual bool	IsEnabled() const = 0;
	virtual void	Reset() = 0;
};
STATIC_ASSERT(sizeof(BSIInputDevice) == 0x8);

class HardwareToActionCode
{
public:
	UInt32			code;

	struct Mapping
	{
		BSFixedString	name;	// 00
		float			timer;	// 08
		UInt32			unk0C;	// 0C
	};
	STATIC_ASSERT(sizeof(Mapping) == 0x10);

	Mapping*	mapping;

	HardwareToActionCode(UInt32 a_key, Mapping* a_mapping)
		: code(a_key), mapping(a_mapping) {}

	bool operator==(const HardwareToActionCode & rhs) const { return code == rhs.code; }
	bool operator==(const UInt32 a_key) const { return code == a_key; }
	operator const UInt64() const { return code; }

	static inline UInt32 GetHash(const UInt32 * key)
	{
		UInt32 hash;
		CalculateCRC32_32(&hash, (UInt32)*key);
		return hash;
	}

	void Dump(void)
	{
		_MESSAGE("\t\tcode: %d", code);
		_MESSAGE("\t\taction: %s", mapping ? mapping->name.c_str() : "");
	}
};

typedef tHashSet<HardwareToActionCode, const UInt32> HardwareToActionTable;

class ActionToHardwareCode
{
public:
	const char* action;
	UInt64		code;

	ActionToHardwareCode(const char* a_key, UInt64 a_code)
		: action(a_key), code(a_code) {}

	bool operator==(const ActionToHardwareCode & rhs) const { return action == rhs.action; }
	bool operator==(const UInt64 a_key) const { return code == a_key; }
	operator const UInt64() const { return code; }

	static inline UInt32 GetHash(const UInt64 * key)
	{
		UInt32 hash;
		CalculateCRC32_64(&hash, (UInt64)*key);
		return hash;
	}

	void Dump(void)
	{
		_MESSAGE("\t\taction: %s", action);
		_MESSAGE("\t\tcode: %d", code);
	}
};

typedef tHashSet<ActionToHardwareCode, const char*> ActionToHardwareTable;

class BSInputDevice : public BSIInputDevice
{
public:
	virtual			~BSInputDevice();

	virtual	bool	GetKeyMapping(UInt32 a_key, BSFixedString& a_mapping) override;
	virtual UInt32	GetMappingKey(BSFixedString a_mapping) override;
	virtual void	Unk_06(void) override;
	virtual bool	IsEnabled() const override;

	UInt64	unk08;							// 08
	SInt32	controllerType;					// 10
	UInt32	pad14;							// 14
	HardwareToActionTable	hardwareMap;	// 18
	ActionToHardwareTable	actionMap;		// 48
};

// 80
class BSTrackedControllerDevice : public BSInputDevice
{
public:
	UInt32	deviceHand;						// 78 - They add one to this when passing to OpenVR as ETrackedControllerRole
	SInt32	trackedDeviceIndex;				// 7C
};

// 150
class BSOpenVRControllerDevice : public BSTrackedControllerDevice
{
public:
	vr_1_0_12::VRControllerState001_t	prevState;	// 80
	vr_1_0_12::VRControllerState001_t	state;		// C0
	UInt64	unk100;
	UInt64	unk108;
	UInt64	unk110;
	UInt64	unk118;
	UInt64	unk120;
	UInt64	unk128;
	UInt32	unk130;
	float	unk134;
	UInt32	unk138;
	float	unk13C;
	UInt32	unk140;
	UInt32	unk144;
	UInt32	unk148;
	UInt32	unk14C;
};

//	00	ButtonEvent
//	01	MouseMoveEvent
//	02	CharEvent
//	03	ThumbstickEvent
//	04	DeviceConnectEvent
//	05	KinectEvent
//	06	VrWandTouchpadPositionEvent
//	07	VrWandTouchpadSwipeEvent

// 18
class InputEvent
{
public:
	enum
	{
		kEventType_Button = 0,
		kEventType_MouseMove,
		kEventType_Char,
		kEventType_Thumbstick,
		kEventType_DeviceConnect,
		kEventType_Kinect,
		kEventType_VrWandTouchpadPositionEvent,
		kEventType_VrWandTouchpadSwipeEvent
	};

	virtual					~InputEvent();
	virtual bool			IsIDEvent();
	virtual BSFixedString *	GetControlID();

//	void			** _vtbl;	// 00
	UInt32			deviceType;	// 08
	UInt32			eventType;	// 0C
	InputEvent		* next;		// 10

	void LogEvent();
};

class IDEvent
{
public:
	BSFixedString	controlID;	// 00
};

// 38
class ButtonEvent : public IDEvent, public InputEvent
{
public:
	virtual					~ButtonEvent();
	virtual bool			IsIDEvent();
	virtual BSFixedString *	GetControlID();

	// 18 -> controlID from IDEvent
	SInt32			keyMask;	// 20 (00000038 when ALT is pressed, 0000001D when STRG is pressed)
	UInt32			pad24;		// 24
	SInt32			flags;		// 28
	UInt32			unk2C;		// 2C 
	float			isDown;		// 30
	float			timer;		// 34 (hold duration)
};

class MouseMoveEvent : public IDEvent, public InputEvent
{
};

// 20
class CharEvent : public InputEvent
{
public:
	UInt32			keyCode;		// 18 (ascii code)
};

class ThumbstickEvent : public IDEvent, public InputEvent
{
public:
	virtual					~ThumbstickEvent();
	virtual bool			IsIDEvent();
	virtual BSFixedString *	GetControlID();

	// 18 -> controlID from IDEvent
	UInt32	keyMask;	// 20 - b for left stick, c for right stick
	UInt32	pad24;		// 24
	float	x;			// 28
	float	y;			// 2C
};

class DeviceConnectEvent : public InputEvent
{
};

class KinectEvent : public IDEvent, public InputEvent
{
};

class VrWandPositionEvent : public IDEvent, public InputEvent
{
public:
	SInt64		unk20;	// 20
	SInt64		unk28;	// 28
	float		x;		// 30
	float		y;		// 34
	UInt32		unk38;	// 38
	UInt32		unk3C;	// 3C
};

class VrWandTouchpadPositionEvent : public VrWandPositionEvent
{
public:

};

class VrWandTouchpadSwipeEvent : public VrWandPositionEvent
{
public:

};

// E8 
class InputEventDispatcher : public EventDispatcher<InputEvent,InputEvent*>
{
public:
	UInt32			unk058;			// 058
	UInt32			pad05C;			// 05C
	BSInputDevice	* devices[kDeviceType_Num];		// 060 

	bool	IsGamepadEnabled(void);

	static InputEventDispatcher* GetSingleton();
};

template <>
class BSTEventSink <InputEvent>
{
public:
	virtual ~BSTEventSink() {}; // todo?
	virtual	EventResult ReceiveEvent(InputEvent ** evn, InputEventDispatcher * dispatcher) = 0;
};

// 128 
class InputManager
{
public:
	enum
	{
		kContext_Gameplay = 0,
		kContext_MenuMode,
		kContext_Console,
		kContext_ItemMenu,
		kContext_Inventory,
		kContext_DebugText,
		kContext_Favorites,
		kContext_Map,
		kContext_Stats,
		kContext_Cursor,
		kContext_Book,
		kContext_DebugOverlay,
		kContext_Journal,
		kContext_TFCMode,
		kContext_MapDebug,
		kContext_Lockpicking,
		kContext_Favor,
		kContext_CraftingMenu,
		kContext_BarterMenu,
		kContext_RaceSexMenu,
		kContext_DialogueMenu,
		kContextCount
	};

	struct InputContext
	{
		// 18
		struct Mapping
		{
			BSFixedString	name;		// 00
			UInt32			buttonID;	// 08
			UInt32			sortIndex;	// 0C
			UInt32			unk10;		// 10
			UInt32			pad14;		// 14
		};

		tArray<Mapping>	deviceMap[kDeviceType_Num];
	};

	struct Unk1
	{
		const char*	unk00;
		UInt32		unk08;
		UInt32		unk0C;
		UInt32		unk10;
		UInt32		unk14;
		const char*	unk18;
	};


	void*			unkPtr000;
	BSTEventSource<void *>	unk008;
	InputContext	* context[kContextCount];
	tArray<Unk1>	unk108;
	tArray<UInt32>	unk120;
	UInt64			unk138;
	UInt8			allowTextInput;
	UInt8			unk121;
	UInt8			unk122;
	UInt8			pad[5];

	static InputManager *	GetSingleton(void);

	static UInt32	GetDeviceOffsetForDevice(UInt32 deviceType);
	static UInt32	GetDeviceTypeFromVR(bool primary);
	static UInt32	GetDeviceTypeFromKeyCode(UInt32 keyCode);
	static UInt32	GetDeviceType();

	UInt8			AllowTextInput(bool allow);

	UInt32			GetMappedKey(BSFixedString name, UInt32 deviceType, UInt32 contextIdx);

	BSFixedString	GetMappedControl(UInt32 buttonID, UInt32 deviceType, UInt32 contextIdx);
};
STATIC_ASSERT(offsetof(InputManager, allowTextInput) == 0x140);

// 30
struct MovementData
{
	NiPoint2	movement;		// 00
	NiPoint2	unk08;			// 08
	float		unk10;			// 10
	float		unk14;			// 14
	float		unk18;			// 18
	float		unk1C;			// 1C
	UInt32		unk20;			// 20
	UInt8		autoRun;		// 24
	UInt8		runMode;		// 25
	UInt8		unk26;			// 26
	bool		fovSlideMode;	// 27
	bool		povScriptMode;	// 28
	bool		povBeastMode;	// 29
	UInt8		unk2A;			// 2A
	UInt8		unk2B;			// 2B
	bool		remapMode;		// 2C
	UInt8		unk2D;			// 2D
	UInt16		unk2E;			// 2E
};
STATIC_ASSERT(sizeof(MovementData) == 0x30);

// 10
class PlayerInputHandler
{
public:
	PlayerInputHandler();
	virtual ~PlayerInputHandler();

	virtual bool CanProcess(InputEvent* a_event);
	virtual void ProcessThumbstick(ThumbstickEvent* a_event, MovementData* a_movementData);
	virtual void ProcessMouseMove(MouseMoveEvent* a_event, MovementData* a_movementData);
	virtual void ProcessButton(ButtonEvent* a_event, MovementData* a_movementData);
	virtual bool Unk_05(InputEvent* inputEvent, void* unk1);
	virtual bool Unk_06(InputEvent* unk1);


	UInt32	unk08;				// 08
	UInt32	pad0C;				// 0C
};

class HeldStateHandler : public PlayerInputHandler
{
public:
	virtual ~HeldStateHandler();

	virtual void Unk_07(InputEvent* unk1);
	virtual void Unk_08(bool unk1);
};

// 1D8 or 1E0
class PlayerControls 
	: public BSTEventSink<InputEvent*>			// 000
	, public BSTEventSink<MenuOpenCloseEvent>	// 008
	, public BSTEventSink<MenuModeChangeEvent>	// 010
	, public BSTEventSink<TESFurnitureEvent>	// 018
{
public:
	virtual			~PlayerControls();

	UInt32			unk020;			// 020
	MovementData	movement;		// 024
	tArray<void*>	unk058;			// 058
	tArray<void*>	unk070;			// 070
	tArray<void *>	unk088;			// 088
	UInt8			unkA0[8];		// 0A0
	UInt64			unkA8;			// 0A8
	float			unkB0[8];		// B0
	UInt32			unkD0[10];		// D0
	UInt8			unkF8[8];		// F8
	float			unk100[20];		// 100
	tArray<void*>	unk150;			// 150
	UInt64			unk168;			// 168

	enum InputHandler
	{
		kInputHandler_Movement,
		kInputHandler_Look,
		kInputHandler_Sprint,
		kInputHandler_ReadyWeapon,
		kInputHandler_AutoMove,
		kInputHandler_ToggleRun,
		kInputHandler_Activate,
		kInputHandler_Jump,
		kInputHandler_Shout,
		kInputHandler_AttackBlock,
		kInputHandler_Run,
		kInputHandler_Sneak,
		kInputHandler_TogglePOV,
		kInputHandler_Teleport,
		kInputHandler_VrSwim,
		kInputHandler_DragonRiding,
		kInputHandler_Num
	};

	PlayerInputHandler* inputHandlers[kInputHandler_Num];	// 170

	static PlayerControls *	GetSingleton(void);

	// used by Hooks_Event
	PlayerControls * ctor_Hook(void);
	MEMBER_FN_PREFIX(PlayerControls);
	DEFINE_MEMBER_FN(ctor, PlayerControls *, 0x0072B900);
};
STATIC_ASSERT(offsetof(PlayerControls, unk058) == 0x58);
STATIC_ASSERT(offsetof(PlayerControls, inputHandlers) == 0x170);

// 90
class MenuControls
{
public:
	virtual			~MenuControls();
	virtual UInt32	Unk_01();

//	void			** _vtbl;		// 00
	BSTEventSink<MenuModeChangeEvent> menuModeEventSink; // 08
	UInt64			unk10;			// 10
	tArray<void*>	arr18;			// 18
	UInt64			unk30[3];		// 30

	MenuEventHandler* clickHandler;	// 48
	MenuEventHandler* directionHandler;	// 50
	MenuEventHandler* consoleOpenHandler;	// 58
	MenuEventHandler* quickSaveLoadHandler;	// 60
	MenuEventHandler* menuOpenHandler;	// 68
	MenuEventHandler* favoritesHandler;	// 70
	MenuEventHandler* screenshotHandler;	// 78

	UInt8			unk80;			// 80
	UInt8			unk81;			// 81
	bool			remapMode;		// 82
	UInt8			unk83;			// 83
	UInt8			pad84[0x90 - 0x84];	// 84

	static MenuControls *	GetSingleton(void);
};
STATIC_ASSERT(offsetof(MenuControls, remapMode) == 0x082);

// 340
class InputStringHolder
{
public:
	void			* unk00;			// 000
	BSFixedString	forward;			// 008 "Forward"
	BSFixedString	back;				// 010 "Back"
	BSFixedString	strafeLeft;			// 018 "Strafe Left"
	BSFixedString	strafeRight;		// 020 "Strafe Right"
	BSFixedString	move;				// 028 "Move"
	BSFixedString	look;				// 030 "Look"
	BSFixedString	activate;			// 038 "Activate"
	BSFixedString	leftAttack;			// 040 "Left Attack/Block"
	BSFixedString	rightAttack;		// 048 "Right Attack/Block"
	BSFixedString	dualAttack;			// 050 "Dual Attack"
	BSFixedString	forceRelease;		// 058 "ForceRelease"
	BSFixedString	pause;				// 060 "Pause"
	BSFixedString	readyWeapon;		// 068 "Ready Weapon"
	BSFixedString	togglePOV;			// 070 "Toggle POV"
	BSFixedString	jump;				// 078 "Jump"
	BSFixedString	journal;			// 080 "Journal"
	BSFixedString	sprint;				// 088 "Sprint"
	BSFixedString	sneak;				// 090 "Sneak"
	BSFixedString	shout;				// 098 "Shout"
	BSFixedString	kinectShout;		// 0A0 "KinectShout"
	BSFixedString	grab;				// 0A8 "Grab"
	BSFixedString	run;				// 0B0 "Run"
	BSFixedString	toggleRun;			// 0B8 "Toggle Always Run"
	BSFixedString	autoMove;			// 0C0 "Auto-Move"
	BSFixedString	quicksave;			// 0C8 "Quicksave"
	BSFixedString	quickload;			// 0D0 "Quickload"
	BSFixedString	newSave;			// 0D8 "NewSave"
	BSFixedString	inventory;			// 0E0 "Inventory"
	BSFixedString	stats;				// 0E8 "Stats"
	BSFixedString	map;				// 0F0 "Map"
	BSFixedString	screenshot;			// 0F8 "Screenshot"
	BSFixedString	multiScreenshot;	// 100 "Multi-Screenshot"
	BSFixedString	console;			// 108 "Console"
	BSFixedString	cameraPath;			// 110 "CameraPath"
	BSFixedString	tweenMenu;			// 118 "Tween Menu"
	BSFixedString	takeAll;			// 120 "Take All"
	BSFixedString	accept;				// 128 "Accept"
	BSFixedString	cancel;				// 130 "Cancel"
	BSFixedString	up;					// 138 "Up"
	BSFixedString	down;				// 140 "Down"
	BSFixedString	left;				// 148 "Left"
	BSFixedString	right;				// 150 "Right"
	BSFixedString	pageUp;				// 158 "PageUp"
	BSFixedString	pageDown;			// 160 "PageDown"
	BSFixedString	pick;				// 168 "Pick"
	BSFixedString	pickNext;			// 170 "PickNext"
	BSFixedString	pickPrevious;		// 178 "PickPrevious"
	BSFixedString	cursor;				// 180 "Cursor"
	BSFixedString	kinect;				// 188 "Kinect"
	BSFixedString	sprintStart;		// 190 "SprintStart"
	BSFixedString	sprintStop;			// 198 "SprintStop"
	BSFixedString	sneakStart;			// 1A0 "sneakStart"
	BSFixedString	sneakStop;			// 1A8 "sneakStop"
	BSFixedString	blockStart;			// 1B0 "blockStart"
	BSFixedString	blockStop;			// 1B8 "blockStop"
	BSFixedString	blockBash;			// 1C0 "blockBash"
	BSFixedString	attackStart;		// 1C8 "attackStart"
	BSFixedString	attackPowerStart;	// 1D0 "attackPowerStart"
	BSFixedString	reverseDirection;	// 1D8 "reverseDirection"
	BSFixedString	unequip;			// 1E0 "Unequip"
	BSFixedString	zoomIn;				// 1E8 "Zoom In"
	BSFixedString	zoomOut;			// 1F0 "Zoom Out"
	BSFixedString	rotateItem;			// 1F8 "RotateItem"
	BSFixedString	leftStick;			// 200 "Left Stick"
	BSFixedString	prevPage;			// 208 "PrevPage"
	BSFixedString	nextPage;			// 210 "NextPage"
	BSFixedString	prevSubPage;		// 218 "PrevSubPage"
	BSFixedString	nextSubPage;		// 220 "NextSubPage"
	BSFixedString	leftEquip;			// 228 "LeftEquip"
	BSFixedString	rightEquip;			// 230 "RightEquip"
	BSFixedString	toggleFavorite;		// 238 "ToggleFavorite"
	BSFixedString	favorites;			// 240 "Favorites"
	BSFixedString	hotkey1;			// 248 "Hotkey1"
	BSFixedString	hotkey2;			// 250 "Hotkey2"
	BSFixedString	hotkey3;			// 258 "Hotkey3"
	BSFixedString	hotkey4;			// 260 "Hotkey4"
	BSFixedString	hotkey5;			// 268 "Hotkey5"
	BSFixedString	hotkey6;			// 270 "Hotkey6"
	BSFixedString	hotkey7;			// 278 "Hotkey7"
	BSFixedString	hotkey8;			// 280 "Hotkey8"
	BSFixedString	quickInventory;		// 288 "Quick Inventory"
	BSFixedString	quickMagic;			// 290 "Quick Magic"
	BSFixedString	quickStats;			// 298 "Quick Stats"
	BSFixedString	quickMap;			// 2A0 "Quick Map"
	BSFixedString	toggleCursor;		// 2A8 "ToggleCursor"
	BSFixedString	wait;				// 2B0 "Wait"
	BSFixedString	click;				// 2B8 "Click"
	BSFixedString	mapLookMode;		// 2C0 "MapLookMode"
	BSFixedString	equip;				// 2C8 "Equip"
	BSFixedString	dropItem;			// 2D0 "DropItem"
	BSFixedString	rotate;				// 2D8 "Rotate"
	BSFixedString	nextFocus;			// 2E0 "NextFocus"
	BSFixedString	prevFocus;			// 2E8 "PreviousFocus"
	BSFixedString	setActiveQuest;		// 2F0 "SetActiveQuest"
	BSFixedString	placePlayerMarker;	// 2F8 "PlacePlayerMarker"
	BSFixedString	xButton;			// 300 "XButton"
	BSFixedString	yButton;			// 308 "YButton"
	BSFixedString	chargeItem;			// 310 "ChargeItem"
	UInt64			unk318;				// 318
	BSFixedString	playerPosition;		// 320 "PlayerPosition"
	BSFixedString	localMap;			// 328 "LocalMap"
	BSFixedString	localMapMoveMode;	// 330 "LocalMapMoveMode"
	BSFixedString	itemZoom;			// 338 "Item Zoom"
	BSFixedString	sneakOrJump;		// 340 "Sneak Or Jump"
	BSFixedString	switchTarget;		// 348 "Switch Target"
	BSFixedString	activeOrFavorites;	// 350 "Activate Or Favorites"
	BSFixedString	teleportOrActivate;	// 358 "Teleport Or Activate"
	BSFixedString	turnLeft;			// 360 "Turn Left"
	BSFixedString	turnRight;			// 368 "Turn Right"
	BSFixedString	sceNavigationMode;	// 370 "Sce Navigation Mode"
	BSFixedString	tabLeft;			// 378 "Tab Left"
	BSFixedString	tabRight;			// 380 "Tab Right"
	BSFixedString	tabByHandedness;	// 388 "Tab By Handedness"
	BSFixedString	changePage;			// 390 "Change Page"
	BSFixedString	leftEquipOrTake;	// 398 "Left Equip Or Take"
	BSFixedString	rightEquipOrTake;	// 3A0 "Right Equip Or Take"
	BSFixedString	takeAllOrStoreOrDropLeft; // 3A8 "Take All Or Store Or Drop Left"
	BSFixedString	takeAllOrStoreOrDropRight; // 3B0 "Take All Or Store Or Drop Right"
	BSFixedString	toggleFavorite2;	// 3B8 "Toggle Favorite"
	BSFixedString	chargeOrUnlock;		// 3C0 "Charge Or Unlock"
	BSFixedString	mapTeleport;		// 3C8 "Map Teleport"
	BSFixedString	mapClick;			// 3D0 "Map Click"
	BSFixedString	toggleOccCulling;	// 3D8 "ToggleOccCulling"
	BSFixedString	deleteSave;			// 3E0 "Delete Save"
	BSFixedString	cancelAlt;			// 3E8 "Cancel Alt"
	BSFixedString	rightStick;			// 3F0 "Right Stick"
	BSFixedString	tweenOrReadyWeapon; // 3F8 "Tween Or Ready Weapon"
	BSFixedString	journalOrWait;		// 400 "Journal Or Wait"

	static InputStringHolder *	GetSingleton(void)
	{
		// F1F9E171ECCBA02B07850342C5C84CAE531026C7+D0
		static RelocPtr<InputStringHolder*> g_inputStringHolder(0x02FEA140);
		return *g_inputStringHolder;
	}
};
STATIC_ASSERT(sizeof(InputStringHolder) == 0x408);

extern RelocPtr<bool> g_isUsingMotionControllers;
extern  RelocPtr<bool> g_leftHandedMode;