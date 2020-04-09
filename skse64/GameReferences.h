#pragma once

#include "skse64/GameFormComponents.h"
#include "skse64/GameForms.h"
#include "skse64/GameEvents.h"
#include "skse64/NiObjects.h"
#include "skse64/GameObjects.h"
#include "GameBSExtraData.h"

class BSAnimationGraphEvent;
class NiNode;
class NiPoint3;
class TESObjectREFR;
class BSFaceGenNiNode;
class BSFaceGenAnimationData;
class PlayerCharacter;
class ImageSpaceModifierInstanceDOF;
class ObjectListItem;
class BSFadeNode;
class BGSLightingShaderProperty;
class BSAnimationGraphManager;
class QueuedFile;
class InventoryEntryData;

class VMClassRegistry;

// TESObjectREFR and child classes

// 10 
class BSHandleRefObject : public NiRefObject
{
public:
	enum
	{
		kMask_RefCount = 0x3FF
	};

	UInt32 GetRefCount() const
	{
		return m_uiRefCount & kMask_RefCount;
	}

	void DecRefHandle()
	{
		if((InterlockedDecrement(&m_uiRefCount) & kMask_RefCount) == 0)
			DeleteThis();
	}
};

// The refHandle is stored in BSHandleRefObject.m_uiRefCount
// Bits:
//	01 - 10	ref count
//	11		isHandleSet
//	12 - ??	handle (seems to be divided in two parts, 0xFFFFF and 0x3F00000)

// Adds entry to lookup table if not yet there
typedef void (* _CreateRefHandleByREFR)(UInt32 * refHandleOut, TESObjectREFR * refr);
extern RelocAddr<_CreateRefHandleByREFR> CreateRefHandleByREFR;

// Note: May set refHandle to 0
typedef bool(*_LookupREFRByHandle)(UInt32 & refHandle, NiPointer<TESObjectREFR> & refrOut);
extern RelocAddr<_LookupREFRByHandle> LookupREFRByHandle;

typedef bool(*_LookupREFRObjectByHandle)(UInt32 & refHandle, NiPointer<BSHandleRefObject> & refrOut);
extern RelocAddr<_LookupREFRObjectByHandle> LookupREFRObjectByHandle;

extern RelocPtr<UInt32> g_invalidRefHandle;

// Try to retrive/create handle. Return invalid handle otherwise.
UInt32 GetOrCreateRefrHandle(TESObjectREFR* refr);

// Place/move reference
typedef void (* _MoveRefrToPosition)(TESObjectREFR* source, UInt32* pTargetHandle, void* parentCell, void* worldSpace, NiPoint3* postion, NiPoint3* rotation);
extern RelocAddr<_MoveRefrToPosition> MoveRefrToPosition;

typedef TESObjectREFR* (* _PlaceAtMe_Native)(VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, TESForm* form, SInt32 count, bool bForcePersist, bool bInitiallyDisabled);
extern RelocAddr<_PlaceAtMe_Native> PlaceAtMe_Native;

typedef void (* _AddItem_Native)(VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent);
extern RelocAddr<_AddItem_Native> AddItem_Native;

// 8 
class IAnimationGraphManagerHolder
{
public:
	virtual ~IAnimationGraphManagerHolder();

	virtual UInt32			Unk_01(void);
	virtual UInt32			Unk_02(void);
	virtual UInt32			Unk_03(void);
	virtual UInt32			Unk_04(void);
	virtual UInt32			Unk_05(void);
	virtual UInt32			Unk_06(void);
	virtual UInt32			Unk_07(void);
	virtual UInt32			Unk_08(void);
	virtual UInt32			Unk_09(void);
	virtual UInt32			Unk_0A(void);
	virtual UInt32			Unk_0B(void);
	virtual UInt32			Unk_0C(void);
	virtual UInt32			Unk_0D(void);
	virtual UInt32			Unk_0E(void);
	virtual UInt32			Unk_0F(void);
	virtual UInt32			Unk_10(void);
	virtual UInt32			Unk_11(void);
	virtual UInt32			Unk_12(void);

//	void	** _vtbl;
};

// 8 
class IPostAnimationChannelUpdateFunctor
{
public:
	virtual ~IPostAnimationChannelUpdateFunctor();
//	void	** _vtbl;
};

// 98 
class TESObjectREFR : public TESForm
{
public:
	enum { kTypeID = kFormType_Reference };

	enum
	{
		kFlag_Harvested = 0x2000,
	};

	// currently none of these have been analyzed
	virtual void	Unk_39(void);
	virtual void	Unk_3A(void);
	virtual void	Unk_3B(void);
	virtual void	Unk_3C(void);
	virtual void	Unk_3D(void);
	virtual void	Unk_3E(void);
	virtual void	Unk_3F(void);
	virtual void	Unk_40(void);
	virtual void	Unk_41(void);
	virtual void	Unk_42(void);
	virtual void	Unk_43(void);
	virtual void	Unk_44(void);
	virtual void	Unk_45(void);
	virtual void	Unk_46(void);
	virtual void	Unk_47(void);
	virtual void	Unk_48(void);
	virtual void	Unk_49(void);
	virtual void	Unk_4A(void);
	virtual void	Unk_4B(void);
	virtual void	Unk_4C(void);
	virtual void	Unk_4D(void);
	virtual void	Unk_4E(void);
	virtual void	Unk_4F(void);
	virtual void	Unk_50(void);
	virtual void	GetStartingPos(NiPoint3 * pos);
	virtual void	Unk_52(void);
	virtual void	Unk_53(void);
	virtual void	Unk_54(void);
	virtual void	Unk_55(void);
	virtual void	Unk_56(void);
	virtual void	Unk_57(void);
	virtual void	Unk_58(void);
	virtual void	Unk_59(void);
	virtual void	Unk_5A(void);
	virtual void	GetMarkerPosition(NiPoint3 * pos);
	virtual void	Unk_5C(void);
	virtual void	Unk_5D(void);
	virtual void	Unk_5E(void);
	virtual void	Unk_5F(void);
	virtual void	Unk_60(void);
	virtual BSFaceGenNiNode *	GetFaceGenNiNode(void);
	virtual void	Unk_62(void);
	virtual BSFaceGenAnimationData *	GetFaceGenAnimationData(void);
	virtual void	Unk_64(void);
	virtual void	Unk_65(void);
	virtual void	Unk_66(void);
	virtual void	Unk_67(void);
	virtual void	Unk_68(void);
	virtual void	Unk_69(void);
	virtual void	Unk_6A(void);
	virtual void 	Unk_6B(void);
	virtual void	Unk_6C(void);
	virtual void	SetNiNode(NiNode * root, UInt32 unk1); // NULL, 1?
	virtual void	Unk_6E(void);
	virtual NiNode	* GetNiRootNode(UInt32 firstPerson);
	virtual NiNode	* GetNiNode(void);	// Root of the skeleton (Get3D)
	virtual void	Unk_71(void);
	virtual void	Unk_72(void);
	virtual void	Unk_73(void);
	virtual void	Unk_74(void);
	virtual void	Unk_75(void);
	virtual void	Unk_76(void);
	virtual void	Unk_77(void);
	virtual void	Unk_78(void);
	virtual void	Unk_79(void);
	virtual void	Unk_7A(void);
	virtual void	Unk_7B(void);
	virtual void	Unk_7C(void);
	virtual void	Unk_7D(void);
	virtual BipedModel* GetBiped(UInt32 weightModel); // 0 Small 1 Large
	virtual BipedModel*	GetBipedSmall(void);
	virtual void	Unk_80(void);
	virtual void	Unk_81(void);
	virtual void	Unk_82(void);
	virtual void	Unk_83(void);
	virtual void	Unk_84(void);
	virtual void	Unk_85(void);
	virtual void	Unk_86(void);
	virtual void	Unk_87(void);
	virtual void	Unk_88(void);
	virtual void	Unk_89(void);
	virtual void	ResetInventory(bool unk);
	virtual void	Unk_8B(void);
	virtual void	Unk_8C(void);
	virtual void	Unk_8D(void);
	virtual void	Unk_8E(void);
	virtual void	Unk_8F(void);
	virtual void	Unk_90(void);
	virtual void	Unk_91(void);
	virtual void	Unk_92(void);
	virtual void	Unk_93(void);
	virtual void	Unk_94(void);
	virtual void	Unk_95(void);
	virtual void	Unk_96(void);
	virtual void	Unk_97(void);
	virtual void	Unk_98(void);
	virtual void	Unk_99(void);
	virtual bool	IsDead(UInt8 unk1); // unk1 = 1 for Actors
	virtual void	Unk_9B(void);

	struct LoadedState
	{
		UInt8	todo[0x68];
		NiNode	* node;	// 68
		// ... probably more
	};

	// parents
	BSHandleRefObject	handleRefObject;	// 20
	BSTEventSink <BSAnimationGraphEvent>	animGraphEventSink;	// 30
	IAnimationGraphManagerHolder			animGraphHolder;	// 38

	// members
	TESForm* baseForm;	// 40
	NiPoint3	rot;	// 48
	NiPoint3	pos;	// 54

	TESObjectCELL	* parentCell;	// 60
	LoadedState		* loadedState;	// 68	
	BaseExtraList	extraData;		// 70
	UInt64			unk88; // 88 - New in SE
	UInt16			unk90;	// 90 - init'd to 100
	UInt8			unk92;	// 92
	UInt8			unk93;	// 93
	UInt32			pad94;	// 94

	UInt32 CreateRefHandle(void);

	void IncRef();
	void DecRef();

	MEMBER_FN_PREFIX(TESObjectREFR);
	DEFINE_MEMBER_FN(GetBaseScale, float, 0x0029E390);
	DEFINE_MEMBER_FN(IsOffLimits, bool, 0x002ABA40);
	// E2B825B476DC5CFBC69E194AA76CB0C049AC78E3+137
	DEFINE_MEMBER_FN(GetWeight, float, 0x002B8760);
	DEFINE_MEMBER_FN(GetReferenceName, const char *, 0x002A78F0);
	DEFINE_MEMBER_FN(GetWorldspace, TESWorldSpace*, 0x002AAE50);
};

STATIC_ASSERT(sizeof(TESObjectREFR) == 0x98);
STATIC_ASSERT(offsetof(TESObjectREFR, extraData) == 0x70);
STATIC_ASSERT(offsetof(TESObjectREFR, loadedState) == 0x68);
STATIC_ASSERT(offsetof(TESObjectREFR::LoadedState, node) == 0x68);

// 08 
class IMovementInterface
{
public:
	virtual ~IMovementInterface();
//	void	** _vtbl;	// 00
};

// 08 
class IMovementState : public IMovementInterface
{
public:
	virtual ~IMovementState();
};

// 10
class ActorState : public IMovementState
{
public:
	virtual ~ActorState();

//	void	** _vtbl;	// 00

	// older versions of this class stored flags in a UInt64
	// this forced the addition of 4 useless padding bytes
	// current and future versions store flags as two UInt32s

	enum {
		kState_Running = 0x40,
		kState_Walking = 0x80,
		kState_Sprinting = 0x100,
		kState_Sneaking = 0x200,
		kState_Swimming = 0x400
	};

	// SE: not changing names of these, they are referenced somewhere else
	UInt32	flags04;	// 08
	UInt32	flags08;	// 0C

	bool IsWeaponDrawn()
	{
		return (flags08 >> 5 & 7) >= 3;
	}
};
STATIC_ASSERT(sizeof(ActorState) == 0x10);

// 2B0 VR: 2B8
class Actor : public TESObjectREFR
{
public:
	enum { kTypeID = kFormType_Character };

	virtual ~Actor();
	virtual void Unk_9C(void);
	virtual void Unk_9D(void);
	virtual void Unk_9E(void);
	virtual void Unk_9F(void);
	virtual void Unk_A0(void);
	virtual void Unk_A1(void);
	virtual void Unk_A2(void);
	virtual void Unk_A3(void);
	virtual void Unk_A4(void);
	virtual void Unk_A5(void);
	virtual void Unk_A6(void);
	virtual void Unk_A7(void);
	virtual void DrawSheatheWeapon(bool draw);
	virtual void Unk_A9(void);
	virtual void Unk_AA(void);
	virtual void Unk_AB(void);
	virtual void Unk_AC(void);
	virtual void Unk_AD(void);
	virtual void Unk_AE(void);
	virtual void Unk_AF(void);
	virtual void Unk_B0(void);
	virtual void Unk_B1(void);
	virtual void Unk_B2(void);
	virtual void Unk_B3(void);
	virtual void Unk_B4(void);
	virtual void Unk_B5(void);
	virtual void Unk_B6(void);
	virtual void Unk_B7(void);
	virtual void Unk_B8(void);
	virtual void Unk_B9(void);
	virtual void Unk_BA(void);
	virtual void Unk_BB(void);
	virtual void Unk_BC(void);
	virtual void Unk_BD(void);
	virtual void Unk_BE(void);
	virtual void Unk_BF(void);
	virtual void Unk_C0(void);
	virtual void Unk_C1(void);
	virtual void Unk_C2(void);
	virtual void Unk_C3(void);
	virtual void Unk_C4(void);
	virtual void Unk_C5(void);
	virtual void Unk_C6(void);
	virtual void Unk_C7(void);
	virtual void Unk_C8(void);
	virtual void Unk_C9(void);
	virtual void Unk_CA(void);
	virtual void Unk_CB(void);
	virtual void Unk_CC(void);
	virtual void Unk_CD(void);
	virtual void Unk_CE(void);
	virtual void Unk_CF(void);
	virtual void Unk_D0(void);
	virtual void Unk_D1(void);
	virtual void Unk_D2(void);
	virtual void Unk_D3(void);
	virtual void Unk_D4(void);
	virtual void Unk_D5(void);
	virtual void Unk_D6(void);
	virtual void Unk_D7(void);
	virtual void Unk_D8(void);
	virtual void Unk_D9(void);
	virtual void Unk_DA(void);
	virtual void Unk_DB(void);
	virtual void Unk_DC(void);
	virtual void Unk_DD(void);
	virtual void Unk_DE(void);
	virtual void Unk_DF(void);
	virtual void Unk_E0(void);
	virtual void Unk_E1(void);
	virtual void Unk_E2(void);
	virtual void Unk_E3(void);
	virtual void Unk_E4(void);
	virtual bool IsInCombat(void);
	virtual void Unk_E6(void);
	virtual void Unk_E7(void);
	virtual void Unk_E8(void);
	virtual void Unk_E9(void);
	virtual void Unk_EA(void);
	virtual void Unk_EB(void);
	virtual void Unk_EC(void);
	virtual void Unk_ED(void);
	virtual void Unk_EE(void);
	virtual void Unk_EF(void);
	virtual void Unk_F0(void);
	virtual void Unk_F1(void);
	virtual void Unk_F2(void);
	virtual void Unk_F3(void);
	virtual void Unk_F4(void);
	virtual void Unk_F5(void);
	virtual void Unk_F6(void);
	virtual void Unk_F7(void);
	virtual void Unk_F8(void);
	virtual void AdvanceSkill(UInt32 skillId, float points, UInt32 unk1, UInt32 unk2);
	virtual void Unk_FA(void);
	virtual void Unk_FB(void);
	virtual void VisitPerks(void); // BGSPerk::FindPerkInRanksVisitor
	virtual void AddPerk(BGSPerk * perk, UInt32 unk1);
	virtual void RemovePerk(BGSPerk * perk);

	// 0C
	class SpellArray
	{
	public:
		SpellItem * Get(UInt32 idx)
		{
			if(idx >= spellCount) return NULL;

			if(allocatedCount & kLocalAlloc)
				return (idx == 0) ? singleSpell : NULL;
			else
				return spells ? spells[idx] : NULL;
		}

		UInt32	Length(void)	{ return spellCount; }

	private:
		enum
	{
			kLocalAlloc = 0x80000000,
		};

		UInt32		allocatedCount;		// 00 - upper bit is set when single-element optimization is used
		union {
			SpellItem	** spells;		// 08
			SpellItem	* singleSpell;	// 08 - used when kLocalAlloc is set
		};
		UInt32		spellCount;		// 10
	};

	enum Flags1 {
		kFlags_AIEnabled = 0x02,
		kFlags_IsPlayerTeammate = 0x4000000
	};
	enum Flags2 {
		kFlags_CanDoFavor = 0x80
	};

	MagicTarget		magicTarget;					// 098
	ActorValueOwner	actorValueOwner;				// 0B0
	ActorState		actorState;						// 0B8
	BSTEventSink<void*>	transformDeltaEvent;		// 0C8 .?AV?$BSTEventSink@VBSTransformDeltaEvent@@@@
	BSTEventSink<void*>	characterMoveFinishEvent;	// 0D0 .?AV?$BSTEventSink@VbhkCharacterMoveFinishEvent@@@@
	IPostAnimationChannelUpdateFunctor	unk0D8;		// 0D8 IPostAnimationChannelUpdateFunctor
	UInt32	flags1;									// 0E0
	float	unk0E4;									// 0E4
	UInt32	unk0E8;									// 0E8
	UInt32	pad0EC;									// 0EC
	ActorProcessManager	* processManager;			// 0F0 
	UInt32	unk0F8;									// 0F8
	UInt32	unk0FC;									// 0FC
	UInt32	unk100;									// 100
	UInt32	unk104;									// 104
	float	unk108;									// 108	- init'd to -1
	UInt32	unk10C;									// 10C
	UInt32	unk110;									// 110
	UInt32	unk114;									// 114
	UInt32	unk118;									// 118
	UInt32	unk11C;									// 11C
	float	unk120;									// 120
	float	unk124;									// 124
	float	unk128;									// 128
	UInt32	unk12C;									// 12C
	UInt64	unk130;									// 130
	UInt64	unk138;									// 138
	void*	unk140;									// 140 ActorMover*
	void*	unk148;									// 148 MovementControllerNPC*
	UInt64	unk150;									// 150
	UInt64	unk158;									// 158
	UInt64	unk160;									// 160
	float	unk168;									// 168
	UInt32	unk16C;									// 16C
	UInt32	unk170;									// 170
	UInt32	unk174;									// 174 - init'd to 50
	UInt32	unk178;									// 178
	UInt32	unk17C;									// 17C - init'd to 7FFFFFFF
	UInt64	unk180;									// 180
	SpellArray	addedSpells;						// 188
	void*	unk1A0;									// 1A0	ActorMagicCaster*
	void*	unk1A8;									// 1A8	ActorMagicCaster*
	UInt64	unk1B0;									// 1B0
	void*	unk1B8;									// 1B8	ActorMagicCaster*
	SpellItem* leftHandSpell;						// 1C0
	SpellItem* rightHandSpell;						// 1C8
	UInt64	unk1D0;									// 1D0
	UInt64	unk1D8;									// 1D8
	TESForm	* equippedShout;						// 1E0
	UInt32	unk1E8;									// 1E8
	UInt32	pad1EC;									// 1EC
	TESRace*	race;								// 1F0
	float	unk1F8;									// 1F8 - init'd to -1
	UInt32	flags2;									// 1FC

													// These two are part of an object
	BSString	unk200;								// 200
	BSString	unk210;								// 210

	UInt64	unk220;									// 220

													// C
	struct Data228
	{
		UInt32	unk0;
		UInt32	unk4;
		UInt32	unk8;
	};

	Data228	unk228;
	Data228	unk234;
	Data228	unk240;
	Data228 unk24C;
	float	unk258;									// 258 - init'd to -1
	UInt32	unk25C;									// 25C
	UInt64	unk260;									// 260
	float	unk268;									// 268
	float	unk26C;									// 26C
	UInt32	unk270;									// 270
	UInt32	unk274;									// 274	
	UInt64	unk278;									// 278
	UInt64	unk280;									// 280
	UInt64	unk288;									// 288
	UInt64	unk290;									// 290
	UInt64	unk298;									// 298
	UInt64	unk2A0;									// 2A0
	UInt64	unk2A8;									// 2A8

	MEMBER_FN_PREFIX(Actor);
	DEFINE_MEMBER_FN(QueueNiNodeUpdate, void, 0x0069C710, bool updateWeight);
	DEFINE_MEMBER_FN(HasPerk, bool, 0x006025A0, BGSPerk * perk);
	DEFINE_MEMBER_FN(GetLevel, UInt16, 0x005DE910);
	DEFINE_MEMBER_FN(SetRace, void, 0x00610000, TESRace*, bool isPlayer);
	DEFINE_MEMBER_FN(UpdateWeaponAbility, void, 0x0063A2A0, TESForm*, BaseExtraList * extraData, bool bLeftHand);
	DEFINE_MEMBER_FN(UpdateArmorAbility, void, 0x0063A230, TESForm*, BaseExtraList * extraData);
	DEFINE_MEMBER_FN(IsHostileToActor, bool, 0x005F0560, Actor * actor);
	DEFINE_MEMBER_FN(ResetAI, void, 0x005E3990, UInt32 unk1, UInt32 unk2);

	TESForm * GetEquippedObject(bool abLeftHand);
	void UpdateSkinColor();
	void UpdateHairColor();

	class FactionVisitor
	{
	public:
		virtual bool Accept(TESFaction * faction, SInt8 rank) = 0;
	};

	// Can contain duplicate entries with different rankings
	bool VisitFactions(FactionVisitor & visitor);
};

STATIC_ASSERT(offsetof(Actor, magicTarget) == 0x98);
STATIC_ASSERT(offsetof(Actor, actorValueOwner) == 0xB0);
STATIC_ASSERT(offsetof(Actor, actorState) == 0xB8);
STATIC_ASSERT(offsetof(Actor, unk0D8) == 0xD8);
STATIC_ASSERT(offsetof(Actor, addedSpells) == 0x188);
STATIC_ASSERT(sizeof(Actor) == 0x2B0);

// 2B0 
class Character : public Actor
{
public:
	enum { kTypeID = kFormType_Character };
};

STATIC_ASSERT(sizeof(Character) == 0x2B0);

// BE0 
class PlayerCharacter : public Character
{
public:
	virtual ~PlayerCharacter();

	// parents
	BSTEventSink <void *>	menuOpenCloseEvent;			// 2B0 .?AV?$BSTEventSink@VMenuOpenCloseEvent@@@@
	BSTEventSink <void *>	menuModeChangeEvent;		// 2B8 .?AV?$BSTEventSink@VMenuModeChangeEvent@@@@
	BSTEventSink <void *>	userEventEnabledEvent;		// 2C0 .?AV?$BSTEventSink@VUserEventEnabledEvent@@@@
	BSTEventSink <void *>	trackedStatsEvent;			// 2C8  BSTEventSink@UTESTrackedStatsEvent@@@@@

	BSTEventSource <void *>	actorCellEventSource;		// 2D0 .?AV?$BSTEventSource@UBGSActorCellEvent@@@@  
														// tArray<void*>: 4 PlayerRegionState, BGSPlayerMusicChanger, CellAcousticSpaceListener, PlayerParentCellListener
	BSTEventSource <void *>	actorDeathEventSource;		// 328 .?AV?$BSTEventSource@UBGSActorDeathEvent@@@@ 
														// tArray<void*>: 1 BGSPlayerMusicChanger
	BSTEventSource <void *>	positionPlayerEventSource;	// 380 .?AV?$BSTEventSource@UPositionPlayerEvent@@@@
														// tArray<void*>: 9 MovementAvoidBoxEventAdapter, GarbaseCollector, Main, MenuTopicManager, TES (85E27728),
														// PathManagerPositionPlayerAdapter, CharacterCollisionMessagePlayerAdapter, PlayerSleepWaitMovementControllerAdapter, SkyrimVM

	enum Node
	{
		kNode_PlayerWorldNode,
		kNode_FollowNode,
		kNode_FollowOffset,
		kNode_HeightOffsetNode,
		kNode_SnapWalkOffsetNode,
		kNode_RoomNode,
		kNode_BlackSphere,
		kNode_UINode,
		kNode_InWorldUIQuadGeometry,
		kNode_UIPointerNode,
		kNode_UIPointerGeometry,
		kNode_DialogueUINode,
		kNode_TeleportDestinationPreview,
		kNode_TeleportDestinationFail,
		kNode_TeleportSprintPreview,
		kNode_SpellOrigin,
		kNode_SpellDestination,
		kNode_ArrowOrigin,
		kNode_ArrowDestination,
		kNode_QuestMarkers,
		kNode_LeftWandNode,
		kNode_LeftWandShakeNode,
		kNode_LeftControllerNode, // This appears to be named after the controller? e.g. LEFT_{indexcontroller}valve_controller_knu_1_0_left
		kNode_Unk1, // NULL
		kNode_LeftWeaponOffsetNode,
		kNode_LeftCrossbowOffsetNode,
		kNode_LeftMeleeWeaponOffsetNode,
		kNode_LeftStaffWeaponOffsetNode,
		kNode_LeftShieldOffsetNode,
		kNode_RightShieldOffsetNode,
		kNode_SecondaryMagicOffsetNode,
		kNode_SecondaryMagicAimNode,
		kNode_SecondaryStaffMagicOffsetNode,
		kNode_RightWandNode,
		kNode_RightWandShakeNode,
		kNode_RightControllerNode, // RIGHT_{indexcontroller}valve_controller_knu_1_0_right
		kNode_Unk2, // NULL
		kNode_RightWeaponOffsetNode,
		kNode_RightCrossbowOffsetNode,
		kNode_RightMeleeWeaponOffsetNode,
		kNode_RightStaffWeaponOffsetNode,
		kNode_PrimaryMagicOffsetNode,
		kNode_PrimaryMagicAimNode,
		kNode_PrimaryStaffMagicOffsetNode,
		kNode_Unk3, // NULL
		kNode_CrosshairParent,
		kNode_CrosshairSecondaryParent,
		kNode_TargetLockParent,
		kNode_HmdNode,
		kNode_LastSyncPos,
		kNode_UprightHmdNode,
		kNode_MapMarkers3D,
		kNode_LeftHandBone, // NPC L Hand [LHnd]
		kNode_RightHandBone, // NPC R Hand [RHnd]
		kNode_LeftCavicle,  // NPC L Cavicle [LClv]
		kNode_RightCavicle, // NPC R Cavicle [RClv]
		kNode_Num
	};
	enum BowNode
	{
		kBowNode_Unk5, // NULL
		kBowNode_Unk6, // NULL
		kBowNode_BowAimNode,
		kBowNode_BowRotationNode,
		kBowNode_ArrowSnapNode,
		kBowNode_ArrowNode,
		kBowNode_ArrowFireNode,
		kBowNode_Unk7, // NULL
		kBowNode_ArrowHoldOffsetNode,
		kBowNode_ArrowHoldNode,
		kBowNode_Num
	};

	UInt64						unk3D8;								// 3D8
	UInt64						unk3E0;								// 3E0
	UInt32						unk3E8;								// 3E8
	NiAVObjectPtr				unk3F0[kNode_Num];					// 3F0
	UInt64						unk530;								// 530
	NiAVObjectPtr				unk538[kBowNode_Num];				// 538
	UInt64						unk608[(0x8B0 - 0x608) >> 3];		// 608
	UInt32						lastRiddenHorseHandle;				// 8B0
	UInt32						pad8B4;								// 8B4
	UInt64						unk8C0[(0xAA0 - 0x8B8) >> 3];		// 8C0
	BGSPerkRanks				addedPerks;							// AA0
	tArray<BGSPerk*>			perks;								// AB8
	tArray<BGSPerk*>			standingStonePerks;					// AD0
	tArray<UInt32>				unkAE0;								// AE0
	UInt64						unkB00[(0xBF8 - 0xB00) >> 3];		// B00
	NiTMap<UInt64,UInt64>		unkBF8;								// BF8
	TESWorldSpace*				currentWorldSpace;					// C18
	UInt64						unkC20[(0xFF0 - 0xC20) >> 3];		// C20
	BSFadeNode*					firstPersonSkeleton;				// FF0
	UInt64						unkFF8[(0x10B0 - 0xFF8) >> 3];		// FF8
	PlayerSkills*				skills;								// 10B0
	UInt64						unk10B8[(0x11F8 - 0x10B8) >> 3];	// 11F8
	UInt32						unk11F8;
	UInt8						unk11FC;
	UInt8						numPerkPoints;						// 11FD
	UInt8						unk11FE;
	UInt8						unk11FF;
	UInt64						unk1200;
	tArray<TintMask*>			tintMasks;							// 1208
	tArray <TintMask *>*		overlayTintMasks;					// 1220
	BGSTextureSet*				texSetB30;							// 1228
	TESRace*					race;								// 1230
	TESRace*					raceAgain;							// 1238
	UInt64						unk1240[(0x12D8 - 0x1240) >> 3];	// 1240


	// Overlayed tints should be the same as original tints
	// occasionally they can have no type so index matching
	// is required to set anything on the tint
	TintMask * GetOverlayTintMask(TintMask * original);

	tArray <TintMask *> * GetTintList()
	{
		if (overlayTintMasks)
			return overlayTintMasks;

		return &tintMasks;
	}

	MEMBER_FN_PREFIX(PlayerCharacter);
	DEFINE_MEMBER_FN(GetNumTints, UInt32, 0x006D8730, UInt32 tintType);
	DEFINE_MEMBER_FN(GetTintMask, TintMask *, 0x006D8580, UInt32 tintType, UInt32 index);
	DEFINE_MEMBER_FN(GetDamage, float, 0x0069C650, InventoryEntryData * pForm);
	DEFINE_MEMBER_FN(GetArmorValue, float, 0x0069C2D0, InventoryEntryData * pForm);
};
STATIC_ASSERT(offsetof(PlayerCharacter, unk3F0) == 0x3F0);
STATIC_ASSERT(offsetof(PlayerCharacter, addedPerks) == 0xAA0);
STATIC_ASSERT(offsetof(PlayerCharacter, perks) == 0xAB8);
STATIC_ASSERT(offsetof(PlayerCharacter, standingStonePerks) == 0xAD0);
STATIC_ASSERT(offsetof(PlayerCharacter, currentWorldSpace) == 0xC18);
STATIC_ASSERT(offsetof(PlayerCharacter, firstPersonSkeleton) == 0xFF0);
STATIC_ASSERT(offsetof(PlayerCharacter, skills) == 0x10B0);
STATIC_ASSERT(offsetof(PlayerCharacter, numPerkPoints) == 0x11FD);
STATIC_ASSERT(offsetof(PlayerCharacter, tintMasks) == 0x1208);
STATIC_ASSERT(offsetof(PlayerCharacter, race) == 0x1230);
STATIC_ASSERT(sizeof(PlayerCharacter) == 0x12D8);

// 140 
class Explosion : public TESObjectREFR
{
	UInt8	unk098[0x140 - 0x98]; // 098 TODO
};

// 1A0 
class ChainExplosion : public Explosion
{
	UInt8	unk1A0[0x1A0 - 0x140]; // 140 TODO
};

// D8 
class Hazard : public TESObjectREFR
{
	enum { kTypeID = kFormType_Hazard };

	UInt64	unk98[8];	// 98 TODO
};

// 1D8
class Projectile : public TESObjectREFR
{
	// A8
	class LaunchData
	{
	public:
		virtual ~LaunchData();

		UInt8	unk08[0xA0]; // 08 TODO                                                             
	};

	UInt8	unk98[0x1D8 - 0x98];	// 98 TODO
};

// 1F8 
class BarrierProjectile : public Projectile
{
	enum { kTypeID = kFormType_BarrierProj };

	UInt32		unk1D8;	// 1D8
	UInt32		pad1DC;	// 1DC	
	UnkArray	unk1E0;	// 1E0
};
STATIC_ASSERT(sizeof(BarrierProjectile) == 0x1F8);

// 240 
class BeamProjectile : public Projectile
{
	enum { kTypeID = kFormType_BeamProj };

	UInt8	unk1D8[0x240 - 0x1D8]; // 1D8 TODO
};

// 218 
class ConeProjectile : public Projectile
{
	enum { kTypeID = kFormType_ConeProj };

	UInt8	unk1D8[0x218 - 0x1D8]; // 1D8 TODO
};

// 1E0 
class FlameProjectile : public Projectile
{
	enum { kTypeID = kFormType_FlameProj };

	UInt64	unk1D8; // 1D8
};

// 1E8 
class GrenadeProjectile : public Projectile
{
	enum { kTypeID = kFormType_Grenade };

	void*	unk1D8;		// 1D8 - Allocated in ctor
	UInt8	unk1E0;		// 1E0
	UInt8	unk1E1[7];	// 1E1 - probably padding
};

// 1E0 
class MissileProjectile  : public Projectile
{
	enum { kTypeID = kFormType_Missile };

	UInt32	unk1D8;		// 1D8
	UInt8	unk1DC;		// 1DC
	UInt8	pad1DD[3];	// 1DD
};

// 1F0 
class ArrowProjectile : public MissileProjectile
{
	enum { kTypeID = kFormType_Arrow };

	UInt64	unk1E0;
	UInt64	unk1E8;
};

// This does alot more, but no idea what it is yet
// ??
class CrosshairRefHandleHolder
{
	UInt32	unk00;					// 00
	UInt32	crosshairRefHandle;		// 04
	// ...

public:

	static CrosshairRefHandleHolder * GetSingleton(void);

	UInt32 CrosshairRefHandle(void) const		{ return crosshairRefHandle; }
};
