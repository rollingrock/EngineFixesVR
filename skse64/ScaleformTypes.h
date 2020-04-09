#pragma once

#include "skse64_common/Utilities.h"
#include "skse64/ScaleformAPI.h"

// not sure why they nest it like this, but whatever
class GRefCountImplCore
{
public:
	GRefCountImplCore() :refCount(1) { }
	virtual ~GRefCountImplCore() { }

//	void	** _vtbl;			// 00
	volatile SInt32	refCount;	// 04

	// redirect new/delete to the scaleform heap
	static void * operator new(std::size_t size)
	{
		return ScaleformHeap_Allocate(size);
	}

	static void * operator new(std::size_t size, const std::nothrow_t &)
	{
		return ScaleformHeap_Allocate(size);
	}

	// placement new
	static void * operator new(std::size_t size, void * ptr)
	{
		return ptr;
	}

	static void operator delete(void * ptr)
	{
		ScaleformHeap_Free(ptr);
	}

	// placement delete
	static void operator delete(void *, void *)
	{
		//
	}

	void	AddRef(void)
	{
		InterlockedIncrement(&refCount);
	}

	void	Release(void)
	{
		if(InterlockedDecrement(&refCount) == 0)
		{
			delete this;
		}
	}
};

class GRefCountImpl : public GRefCountImplCore
{
public:
	GRefCountImpl() { }
	virtual ~GRefCountImpl() { }
};

class GRefCountBaseStatImpl : public GRefCountImpl
{
public:
	GRefCountBaseStatImpl() { }
	virtual ~GRefCountBaseStatImpl() { }
};

class GRefCountBase : public GRefCountBaseStatImpl
{
public:
	GRefCountBase() { }
	virtual ~GRefCountBase() { }
};

#pragma warning (push)
#pragma warning (disable : 4200)

class GString
{
public:
	GString()	{ }
	GString(const char * string)	{ CALL_MEMBER_FN(this, ctor)(string); }
	~GString()	{ }

	enum
	{
		kHeapInfoMask =	3,
	};

	struct Data
	{
		UInt32	len;
		SInt32	refCount;
		char	data[0];

		void	IncRef(void);
		void	Release(void);
	};

	union
	{
		Data	* ptr;		// do not use directly, clear lower 3 bits first
		UInt32	heapInfo;
	} data;

	Data *	GetData(void);
	UInt32	GetHeapInfo(void);

	void	Destroy(void);

	MEMBER_FN_PREFIX(GString);
	DEFINE_MEMBER_FN(ctor, GString *, 0x00F2DA40, const char * string);
	
};

#pragma warning (pop)

template <typename T>
class GRect
{
public:
	T	left;
	T	top;
	T	right;
	T	bottom;
};

class GMatrix3D
{
public:
	float	m[4][4];
};

typedef GRect<float>	GRectF;

template <class T>
class GPoint
{
public:
	enum class BoundsType { kMin, kMax };

	// members
	T	x;	// 00
	T	y;	// ??
};

typedef GPoint<float> GPointF;
typedef GPoint<double> GPointD;

template <typename T>
class GArray
{
public:
	T * values;			// 00
	UInt64	size;		// 08
	UInt64	capacity;	// 10
	// ### todo
	MEMBER_FN_PREFIX(GArray);
	DEFINE_MEMBER_FN(AddArgument, void, 0x00000000, T * value); // This function got inlined
};

class GColor
{
public:
	struct RGB32
	{
		UInt8	blue;	// 0
		UInt8	green;	// 1
		UInt8	red;	// 2
		UInt8	alpha;	// 3
	};
	STATIC_ASSERT(sizeof(RGB32) == 0x4);


	union ColorData
	{
		RGB32	channels;
		UInt32	raw;
	};
	STATIC_ASSERT(sizeof(ColorData) == 0x4);


	// members
	ColorData colorData;	// 0
};
STATIC_ASSERT(sizeof(GColor) == 0x4);

class GViewport
{
public:
	enum
	{
		kFlags_IsRenderTexture    = 1 << 0,
		kFlags_AlphaComposite     = 1 << 1,
		kFlags_UseScissorRect     = 1 << 2,
		kFlags_NoSetState         = 1 << 3,
		kFlags_RenderTextureAlpha = kFlags_IsRenderTexture|kFlags_AlphaComposite
	};

	SInt32	bufferWidth;
	SInt32	bufferHeight;
	SInt32	left;
	SInt32	top;
	SInt32	width;
	SInt32	height;
	SInt32	scissorLeft;
	SInt32	scissorTop;
	SInt32	scissorWidth;
	SInt32	scissorHeight;
	float	scale;
	float	aspectRatio;
	UInt32	flags;
};

class GSysAllocBase
{
public:
	virtual ~GSysAllocBase();

	virtual void Unk_01(void);
	virtual void Unk_02(void);
};

class GSysAlloc : public GSysAllocBase
{
public:
	virtual ~GSysAlloc();
};

class ScaleformAllocator : public GSysAlloc
{
public:
	virtual ~ScaleformAllocator();

	virtual void Unk_03(void);
	virtual void Unk_04(void);
};

class GFxEvent
{
public:
	enum EventType
	{
		kNone,
		kMouseMove,
		kMouseDown,
		kMouseUp,
		kMouseWheel,
		kKeyDown,
		kKeyUp,
		kSceneResize,
		kSetFocus,
		kKillFocus,
		kDoShowMouse,
		kDoHideMouse,
		kDoSetMouseCursor,
		kCharEvent,
		kIMEEvent
	};

	// members
	UInt32 type;	// 0
};
STATIC_ASSERT(sizeof(GFxEvent) == 0x4);

class GFxSpecialKeysState
{
public:
	enum class Key
	{
		kShiftPressed = 1 << 0,
		kCtrlPressed = 1 << 1,
		kAltPressed = 1 << 2,
		kCapsToggled = 1 << 3,
		kNumToggled = 1 << 4,
		kScrollToggled = 1 << 5,

		kInitializedBit = 1 << 7,
		kInitializedMask = 0xFF
	};


	UInt8 states;	// 0
};

class GFxKey
{
public:
	enum Code : UInt32
	{
		kVoidSymbol = 0,

		// A through Z and numbers 0 through 9.
		kA = 65,
		kB,
		kC,
		kD,
		kE,
		kF,
		kG,
		kH,
		kI,
		kJ,
		kK,
		kL,
		kM,
		kN,
		kO,
		kP,
		kQ,
		kR,
		kS,
		kT,
		kU,
		kV,
		kW,
		kX,
		kY,
		kZ,
		kNum0 = 48,
		kNum1,
		kNum2,
		kNum3,
		kNum4,
		kNum5,
		kNum6,
		kNum7,
		kNum8,
		kNum9,

		// Numeric keypad.
		kKP_0 = 96,
		kKP_1,
		kKP_2,
		kKP_3,
		kKP_4,
		kKP_5,
		kKP_6,
		kKP_7,
		kKP_8,
		kKP_9,
		kKP_Multiply,
		kKP_Add,
		kKP_Enter,
		kKP_Subtract,
		kKP_Decimal,
		kKP_Divide,

		// Function keys.
		kF1 = 112,
		kF2,
		kF3,
		kF4,
		kF5,
		kF6,
		kF7,
		kF8,
		kF9,
		kF10,
		kF11,
		kF12,
		kF13,
		kF14,
		kF15,

		// Other keys.
		kBackspace = 8,
		kTab,
		kClear = 12,
		kReturn,
		kShift = 16,
		kControl,
		kAlt,
		kPause,
		kCapsLock = 20, // Toggle
		kEscape = 27,
		kSpace = 32,
		kPageUp,
		kPageDown,
		kEnd = 35,
		kHome,
		kLeft,
		kUp,
		kRight,
		kDown,
		kInsert = 45,
		kDelete,
		kHelp,
		kNumLock = 144, // Toggle
		kScrollLock = 145, // Toggle

		kSemicolon = 186,
		kEqual = 187,
		kComma = 188, // Platform specific?
		kMinus = 189,
		kPeriod = 190, // Platform specific?
		kSlash = 191,
		kBar = 192,
		kBracketLeft = 219,
		kBackslash = 220,
		kBracketRight = 221,
		kQuote = 222,

		kOEM_AX = 0xE1,  //  'AX' key on Japanese AX kbd
		kOEM_102 = 0xE2,  //  "<>" or "\|" on RT 102-key kbd.
		kICO_HELP = 0xE3,  //  Help key on ICO
		kICO_00 = 0xE4,  //  00 key on ICO

		kTotal
	};
};
STATIC_ASSERT(sizeof(GFxKey) == 0x1);

class GFxKeyboardState : public GRefCountBase
{
public:
	class IListener
	{
	public:
		virtual ~IListener() = default;																								// 00

		// add
		virtual void OnKeyDown(void* a_sc, int a_code, UInt8 a_ascii, UInt32 a_charCode, UInt8 a_keyboardIndex) = 0;	// 01
		virtual void OnKeyUp(void* a_sc, int a_code, UInt8 a_ascii, UInt32 a_charCode, UInt8 a_keyboardIndex) = 0;		// 02
		virtual void Update(int a_code, UInt8 a_ascii, UInt32 a_charCode, UInt8 a_keyboardIndex) = 0;								// 03
	};
	STATIC_ASSERT(sizeof(IListener) == 0x8);


	class KeyQueue
	{
	public:
		enum
		{
			kKeyQueueSize = 100
		};


		struct KeyRecord
		{
			UInt32				charCode;			// 00
			UInt16				code;				// 04
			UInt16				pad06;				// 06
			GFxEvent::EventType	event;				// 08
			UInt8				ascii;				// 0C
			GFxSpecialKeysState	specialKeysState;	// 0D
			UInt8				pad0E;				// 0E
			UInt8				pad0F;				// 0F
		};
		STATIC_ASSERT(sizeof(KeyRecord) == 0x10);


		// members
		KeyRecord	buffer[kKeyQueueSize];	// 000
		UInt32		putIdx;					// 640
		UInt32		getIdx;					// 644
		UInt32		count;					// 648
	};
	STATIC_ASSERT(sizeof(KeyQueue) == 0x64C);


	// members
	IListener*	listener;						// 010
	KeyQueue	keyQueue;						// 018
	UInt8		keyboardIndex;					// 664
	UInt8		keymap[GFxKey::kTotal / 8 + 1];	// 665
	bool		toggled[3];						// 682
	UInt8		pad685;							// 685
	UInt8		pad686;							// 686
	UInt8		pad687;							// 687
};
STATIC_ASSERT(sizeof(GFxKeyboardState) == 0x688);