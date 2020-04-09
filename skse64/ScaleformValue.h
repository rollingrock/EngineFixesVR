#pragma once

#include "skse64/ScaleformTypes.h"
#include "skse64_common/Utilities.h"

class GFxMovieRoot;

// 10
class GFxValue
{
public:
	GFxValue()
		:objectInterface(NULL), type(kType_Undefined) { }
	~GFxValue();

	enum
	{
		kType_Undefined = 0,
		kType_Null,
		kType_Bool,
		kType_Number,
		kType_String,
		kType_WideString,
		kType_Object,
		kType_Array,
		kType_DisplayObject,
		kType_Function,

		kTypeFlag_Managed = 1 << 6,

		kMask_Type = 0x8F,	// not sure why it checks the top bit
	};

	union Data
	{
		double			number;
		bool			boolean;
		const char		* string;
		const char		** managedString;
		const wchar_t	* wideString;
		const wchar_t	** managedWideString;
		void			* obj;
	};

	//#pragma pack (push, 8)
	class DisplayInfo
	{
	public:
		DisplayInfo() : _varsSet(0) {}
		enum
		{
			kChange_x = (1 << 0),
			kChange_y = (1 << 1),
			kChange_rotation = (1 << 2),
			kChange_xscale = (1 << 3),
			kChange_yscale = (1 << 4),
			kChange_alpha = (1 << 5),
			kChange_visible = (1 << 6),
			kChange_z = (1 << 7),
			kChange_xrotation = (1 << 8),
			kChange_yrotation = (1 << 9),
			kChange_zscale = (1 << 10),
			kChange_FOV = (1 << 11),
			kChange_projMatrix3D = (1 << 12),
			kChange_viewMatrix3D = (1 << 13)
		};

		double		_x;
		double		_y;
		double		_rotation;
		double		_xScale;
		double		_yScale;
		double		_alpha;
		bool		_visible;
		double		_z;
		double		_xRotation;
		double		_yRotation;
		double		_zScale;
		double		_perspFOV;
		GMatrix3D	_viewMatrix3D;
		GMatrix3D	_perspectiveMatrix3D;
		UInt16		_varsSet;

		void SetX(double x) { SetFlags(kChange_x); _x = x; }
		void SetY(double y) { SetFlags(kChange_y); _y = y; }
		void SetRotation(double degrees) { SetFlags(kChange_rotation); _rotation = degrees; }
		void SetXScale(double xscale) { SetFlags(kChange_xscale); _xScale = xscale; }
		void SetYScale(double yscale) { SetFlags(kChange_yscale); _yScale = yscale; }
		void SetAlpha(double alpha) { SetFlags(kChange_alpha); _alpha = alpha; }
		void SetVisible(bool visible) { SetFlags(kChange_visible); _visible = visible; }
		void SetZ(double z) { SetFlags(kChange_z); _z = z; }
		void SetXRotation(double degrees) { SetFlags(kChange_xrotation); _xRotation = degrees; }
		void SetYRotation(double degrees) { SetFlags(kChange_yrotation); _yRotation = degrees; }
		void SetZScale(double zscale) { SetFlags(kChange_zscale); _zScale = zscale; }
		void SetFOV(double fov) { SetFlags(kChange_FOV); _perspFOV = fov; }
		void SetProjectionMatrix3D(const GMatrix3D *pmat)
		{
			if (pmat) {
				SetFlags(kChange_projMatrix3D);
				_perspectiveMatrix3D = *pmat;
			}
			else
				ClearFlags(kChange_projMatrix3D);
		}
		void SetViewMatrix3D(const GMatrix3D *pmat)
		{
			if (pmat) {
				SetFlags(kChange_viewMatrix3D);
				_viewMatrix3D = *pmat;
			}
			else
				ClearFlags(kChange_viewMatrix3D);
		}

		// Convenience functions
		void SetPosition(double x, double y) { SetFlags(kChange_x | kChange_y); _x = x; _y = y; }
		void SetScale(double xscale, double yscale) { SetFlags(kChange_xscale | kChange_yscale); _xScale = xscale; _yScale = yscale; }

		void SetFlags(UInt32 flags) { _varsSet |= flags; }
		void ClearFlags(UInt32 flags) { _varsSet &= ~flags; }
	};
	//#pragma pack (pop)

	// 4
	class ObjectInterface
	{
	public:
		GFxMovieRoot	* root;

		MEMBER_FN_PREFIX(ObjectInterface);
		DEFINE_MEMBER_FN(HasMember, bool, 0x00F27170, void * obj, const char * name, bool isDisplayObj);
		DEFINE_MEMBER_FN(SetMember, bool, 0x00F296F0, void * obj, const char * name, GFxValue * value, bool isDisplayObj);
		DEFINE_MEMBER_FN(DeleteMember, bool, 0x00F25FE0, void * obj, const char * name, bool isDisplayObj);
		DEFINE_MEMBER_FN(GetMember, bool, 0x00F26CA0, void * obj, const char * name, GFxValue * value, bool isDisplayObj);
		DEFINE_MEMBER_FN(Invoke, bool, 0x00F273B0, void * obj, GFxValue * result, const char * name, GFxValue * args, UInt32 numArgs, bool isDisplayObj);
		DEFINE_MEMBER_FN(AttachMovie, bool, 0x00F24EF0, void * obj, GFxValue * value, const char * symbolName, const char * instanceName, SInt32 depth, void * initArgs);
		DEFINE_MEMBER_FN(PushBack, bool, 0x00F27E50, void * obj, GFxValue * value);
		DEFINE_MEMBER_FN(SetText, bool, 0x00F29840, void * obj, const char * text, bool html);
		//DEFINE_MEMBER_FN(PopBack, bool, 0x00000000, void * obj, GFxValue * value);
		DEFINE_MEMBER_FN(GetArraySize, UInt32, 0x00F26720, void * obj);
		//DEFINE_MEMBER_FN(SetArraySize, bool, 0x00000000, void * obj, UInt32 size);
		DEFINE_MEMBER_FN(GetElement, bool, 0x00F26A80, void * obj, UInt32 index, GFxValue * value);
		//DEFINE_MEMBER_FN(SetElement, bool, 0x00000000, void * obj, UInt32 index, GFxValue * value);
		DEFINE_MEMBER_FN(GotoLabeledFrame, bool, 0x00F270C0, void * obj, const char * frameLabel, bool stop);
		//DEFINE_MEMBER_FN(GotoFrame, bool, 0x00000000, void * obj, UInt32 frameNumber, bool stop);
		DEFINE_MEMBER_FN(GetDisplayInfo, bool, 0x00F26770, void * obj, DisplayInfo * displayInfo);
		DEFINE_MEMBER_FN(SetDisplayInfo, bool, 0x00F28900, void * obj, DisplayInfo * displayInfo);

		DEFINE_MEMBER_FN(AddManaged_Internal, void, 0x00F27BD0, GFxValue * value, void * obj);
		DEFINE_MEMBER_FN(ReleaseManaged_Internal, void, 0x00F27C30, GFxValue * value, void * obj);
	};

	ObjectInterface	* objectInterface;	// 00
	UInt32			type;				// 08
	Data			data;				// 10

	GFxValue(const GFxValue& a_rhs) :
		objectInterface(nullptr),
		type(a_rhs.type)
	{
		data = a_rhs.data;
		if (a_rhs.IsManaged()) {
			AddManaged();
			objectInterface = a_rhs.objectInterface;
		}
	}
	GFxValue(GFxValue&& a_rhs) :
		objectInterface(std::move(a_rhs.objectInterface)),
		type(std::move(a_rhs.type))
	{
		a_rhs.objectInterface = 0;
		a_rhs.type = kType_Undefined;

		data.obj = std::move(a_rhs.data.obj);
		a_rhs.data.obj = 0;
	}
	GFxValue&	operator=(const GFxValue& a_rhs)
	{
		if (this == &a_rhs) {
			return *this;
		}

		if (IsManaged()) {
			CleanManaged();
		}

		type = a_rhs.type;
		data = a_rhs.data;
		if (a_rhs.IsManaged()) {
			AddManaged();
			objectInterface = a_rhs.objectInterface;
		}

		return *this;
	}
	GFxValue&	operator=(GFxValue&& a_rhs)
	{
		if (this == &a_rhs) {
			return *this;
		}

		objectInterface = std::move(a_rhs.objectInterface);
		a_rhs.objectInterface = 0;

		type = std::move(a_rhs.type);
		a_rhs.type = kType_Undefined;

		data.obj = std::move(a_rhs.data.obj);
		a_rhs.data.obj = 0;
		return *this;
	}

	UInt32	GetType(void) const { return type & kMask_Type; }
	bool	IsManaged(void) const { return (type & kTypeFlag_Managed) != 0; }
	void	CleanManaged(void);
	void	AddManaged(void);

	bool	IsObject(void) const { return GetType() == kType_Object; }
	bool	IsDisplayObject(void) const { return GetType() == kType_DisplayObject; }

	bool			GetBool(void) const;
	const char *	GetString(void) const;
	const wchar_t *	GetWideString(void) const;
	double			GetNumber(void) const;

	void	SetUndefined(void);
	void	SetNull(void);
	void	SetBool(bool value);
	void	SetNumber(double value);
	void	SetString(const char * value);
	void	SetWideString(const wchar_t * value);

	UInt32	GetArraySize();
	bool	GetElement(UInt32 index, GFxValue * value);
	bool	HasMember(const char * name);
	bool	SetMember(const char * name, GFxValue * value);
	bool	GetMember(const char * name, GFxValue * value);
	bool	DeleteMember(const char * name);
	bool	Invoke(const char * name, GFxValue * result, GFxValue * args, UInt32 numArgs);
	bool	PushBack(GFxValue * value);
	bool	GetDisplayInfo(DisplayInfo * displayInfo);
	bool	SetDisplayInfo(DisplayInfo * displayInfo);
	bool	SetText(const char * text, bool html);
};
