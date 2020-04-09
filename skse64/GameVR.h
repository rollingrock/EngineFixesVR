#pragma once

#include "GameEvents.h"
#include "NiTypes.h"
#include "NiTextures.h"
#include "openvr_1_0_12.h"

struct VROverlayChange
{

};

struct VRDeviceConnectionChange
{

};

struct VRResetHMDHeight
{

};

// 110
class BSVRInterface
	: public BSTEventSource<VROverlayChange>
	, public BSTEventSource<VRDeviceConnectionChange>
	, public BSTEventSource<VRResetHMDHeight>
{
public:
	virtual ~BSVRInterface() = 0;

	// This is ETrackedControllerRole - 1
	enum BSControllerHand
	{
		kControllerHand_Left,
		kControllerHand_Right
	};

	virtual void Shutdown() = 0;
	virtual vr_1_0_12::IVRCompositor* GetCompositor() = 0;
	virtual void Unk_03() = 0;
	virtual void Submit(vr_1_0_12::EVREye eye, const vr_1_0_12::Texture_t* texture) = 0;
	virtual void SetStanding() = 0; // Sets tracking space to standing
	virtual void SetSeated() = 0; // Sets tracking space to seated
	virtual void GetProjectionMatrix(void*) = 0;
	virtual void Unk_08() = 0;
	virtual void Unk_09() = 0;
	virtual void Unk_0A() = 0;
	virtual void PollNextEvent() = 0;
	virtual void Unk_0C() = 0;
	virtual vr_1_0_12::TrackedDeviceIndex_t GetTrackedDeviceHand(BSControllerHand hand) = 0;
	virtual void TriggerHapticPulse(BSControllerHand hand, float duration) = 0; // Duration is multiplied by 0x1417E6E50 which is default 3999.0
	virtual void Unk_0F() = 0;
	virtual void Unk_10() { };
	virtual void Unk_11() = 0;
	virtual void Unk_12() = 0;
	virtual void Unk_13() = 0;
	virtual void Unk_14() { };
	virtual void Unk_15() = 0;

	enum ControllerTypes
	{
		kControllerType_Vive = 0,
		kControllerType_Oculus,
		kControllerType_WindowsMR
	};
	
	// This actually gives a number depending on what the Tracking system is returned as
	// when a controller type is simulated the underlying tracking system is changed by openvr before hitting the game
	virtual SInt32 GetControllerType() = 0; // Gets unk398
	virtual void Unk_17() = 0; // Creates the controller meshes?
};
STATIC_ASSERT(sizeof(BSVRInterface) == 0x110);

// 408
class BSOpenVR : public BSVRInterface
{
public:

	// Same structure as openvr_1_0_12.h just with public access
	struct COpenVRContext
	{
		vr_1_0_12::IVRSystem			*m_pVRSystem;
		vr_1_0_12::IVRChaperone			*m_pVRChaperone;
		vr_1_0_12::IVRChaperoneSetup	*m_pVRChaperoneSetup;
		vr_1_0_12::IVRCompositor		*m_pVRCompositor;
		vr_1_0_12::IVROverlay			*m_pVROverlay;
		vr_1_0_12::IVRResources			*m_pVRResources;
		vr_1_0_12::IVRRenderModels		*m_pVRRenderModels;
		vr_1_0_12::IVRExtendedDisplay	*m_pVRExtendedDisplay;
		vr_1_0_12::IVRSettings			*m_pVRSettings;
		vr_1_0_12::IVRApplications		*m_pVRApplications;
		vr_1_0_12::IVRTrackedCamera		*m_pVRTrackedCamera;
		vr_1_0_12::IVRScreenshots		*m_pVRScreenshots;
		vr_1_0_12::IVRDriverManager		*m_pVRDriverManager;
	};

	UInt64							unk110;				// 110 - Compositor Initialized?
	void*							unk118;				// 118
	UInt64							unk120;				// 120
	float							unk128[(0x190 - 0x128) >> 2];	// 128
	COpenVRContext					vrContext;			// 190
	vr_1_0_12::VROverlayHandle_t	inputOverlay;		// 1F8 - Overlay handle for Input
	UInt64							unk200;				// 200
	vr_1_0_12::IVRSystem*			vrSystem;			// 208 - vrclient_x64 - IVRSystem_019
	void*							unk210;				// 210
	UInt64							unk218;				// 218
	void*							unk220;				// 220
	UInt64							unk228;				// 228
	NiTexturePtr					unk230;				// 230 "SIMPLE_NORMAL_MAP" NiSourceTexture

	// 40
	struct Data238
	{
		UInt64	unk00;
		UInt64	unk08;
		UInt64	unk10;
		UInt64	unk18;
		UInt64	unk20;
		UInt64	unk28;
		UInt64	unk30;
		UInt64	unk38;
	};

	Data238					unk238[4];			// 238
	UInt64					unk338[(0x388 - 0x338) >> 3];
	NiPointer<NiNode>		controller[2];		// 388 - Left, Right
	UInt32					unk398;				// 398
	NiTransform				leftEye;			// 39C
	NiTransform				rightEye;			// 3D0

	DEFINE_MEMBER_FN_0(PollNextEvent_Internal, void, 0x00C53E30);
};
STATIC_ASSERT(offsetof(BSOpenVR, controller) == 0x388);
STATIC_ASSERT(offsetof(BSOpenVR, leftEye) == 0x39C);
STATIC_ASSERT(offsetof(BSOpenVR, rightEye) == 0x3D0);
STATIC_ASSERT(sizeof(BSOpenVR) == 0x408);

extern RelocPtr<BSOpenVR *> g_openVR;


typedef NiTransform&(*_HmdMatrixToNiTransform)(NiTransform& transform, const vr_1_0_12::HmdMatrix34_t& hmdMatrix);
extern RelocAddr<_HmdMatrixToNiTransform> HmdMatrixToNiTransform;