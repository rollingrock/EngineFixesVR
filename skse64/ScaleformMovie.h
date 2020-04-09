#pragma once

#include "skse64/ScaleformTypes.h"

class GFxValue;
class GFxFunctionHandler;

class GFxMovieDef;
class GColor;
class GFxEvent;
class GFxValue;
class GMatrix3D;
class GMemoryHeap;
class GPoint3F;
class GViewport;
struct GStatBag;

class GFxMovieView : public GRefCountBase
{
public:
	GFxMovieView();
	virtual ~GFxMovieView();

	enum Result
	{
		kProcessed = 0,
		kAbort,
		kNotProcessed
	};

	virtual GFxMovieDef*	GetMovieDef() const;	// 01 - pure - Obtains the movie definition that created this instance.
	virtual UInt32			GetCurrentFrame() const; // 02 - pure - Obtains the currently active frame of the movie, starting with 0.
	virtual bool			HasLooped() const;	// 03 - pure - Determines whether the movie clip has looped past its last frame.
	virtual void			GotoFrame(UInt32 a_frameNumber);	// 04 - pure - Moves a playhead to a specified frame.
	virtual bool			GotoLabeledFrame(const char* a_label, SInt32 a_offset = 0);	// 05 - pure - Moves a playhead to a specified frame identified by a label.
	virtual void			SetPlayState(UInt32 a_state);	// 06 - pure - Changes playback state of the movie, allowing animation to be paused and resumed.
	virtual UInt32		GetPlayState() const;	// 07 - pure - Obtains the play state of the movie clip, currently either Playing or Stopped.
	virtual void			SetVisible(bool a_visible);	// 08 - pure - Sets the visibility state of a movie clip.
	virtual bool			GetVisible() const;	// 09 - pure - Obtains the visibility state of a movie clip.
	virtual bool			IsAvailable(const char* a_pathToVar) const;
	virtual void			CreateString(GFxValue * value, const char * str);
	virtual void			CreateWideString(GFxValue * value, const wchar_t * str);
	virtual void			CreateObject(GFxValue * value, const char * className = NULL, GFxValue * args = NULL, UInt32 numArgs = 0);
	virtual void			CreateArray(GFxValue * value);
	virtual void			CreateFunction(GFxValue * value, GFxFunctionHandler * callback, void * refcon = NULL);
	virtual void			SetVariable(const char * name, GFxValue * value, UInt32 flags);
	virtual bool			GetVariable(GFxValue * value, const char * name);
	virtual bool			SetArray(UInt32 type, const char * name, UInt32 offset, void * buf, UInt32 len, UInt32 flags);
	virtual bool			ResizeArray(const char * name, UInt32 len, UInt32 flags);
	virtual UInt32			GetArrayLen(const char * name);	// 050
	virtual bool			GetArray(UInt32 type, const char * name, UInt32 offset, void * buf, UInt32 len);
	virtual bool			Invoke(const char * name, GFxValue * result, GFxValue * args, UInt32 numArgs);
	virtual bool			Invoke(const char * name, GFxValue * result, const char * fmt, ...);
	virtual bool			Invoke_v(const char * name, GFxValue * result, const char * fmt, va_list args);	// 060
	virtual void			SetViewport(const GViewport& a_viewDesc);	// 19 - pure - Sets the render-target surface viewport to which the movie is scaled.
	virtual void			GetViewport(GViewport* a_viewDesc) const;	// 1A - pure - Obtains the render-target surface viewport to which the movie is scaled.
	virtual void			SetViewScaleMode(UInt32 a_type);	// 1B - pure - Sets the current scale mode for the movie.
	virtual UInt32			GetViewScaleMode() const;	// 1C - pure - Obtains the current scale mode for the movie.
	virtual void			SetViewAlignment(UInt32 a_type);	// 1D - pure - Sets the current alignment mode for the movie.
	virtual UInt32			GetViewAlignment() const;	// 1E - pure - Gets the current alignment mode for the movie.
	virtual GRectF			GetVisibleFrameRect() const;	// 1F - pure - Obtains the currently visible rectangle.
	virtual void			SetPerspective3D(const GMatrix3D& a_projMatIn);	// 20 - pure
	virtual void			SetView3D(const GMatrix3D& a_viewMatIn);	// 21 - pure
	virtual GRectF			GetSafeRect() const;	// 22 - pure
	virtual void			SetSafeRect(const GRectF& a_rect);	// 23 - pure
	virtual void			Restart();	// 24 - pure
	virtual float			Advance(float a_deltaT, UInt32 a_frameCatchUpCount = 2);	// 25 - pure
	virtual void			Display();	// 26 - pure
	virtual void			DisplayPrePass();	// 27 - pure
	virtual void			SetPause(bool a_pause);	// 28 - pure
	virtual bool			IsPaused() const;	// 29 - pure
	virtual void			SetBackgroundColor(const GColor a_bgColor);	// 2A - pure
	virtual void			SetBackgroundAlpha(float a_alpha);	// 2B - pure
	virtual float			GetBackgroundAlpha() const;	// 2C - pure
	virtual UInt32			HandleEvent(const GFxEvent& a_event);	// 2D - pure
	virtual void			GetMouseState(UInt32 a_mouseIndex, float* a_x, float* a_y, UInt32* a_buttons);	// 2E - pure
	virtual void			NotifyMouseState(float a_x, float a_y, UInt32 a_buttons, UInt32 a_mouseIndex = 0);	// 2F - pure
	virtual bool			HitTest(float a_x, float a_y, UInt32 a_testCond, UInt32 a_controllerIdx = 0);	// 30 - pure
	virtual bool			HitTest3D(GPoint3F* a_ptOut, float a_x, float a_y, UInt32 a_controllerIdx = 0);	// 31 - pure
	virtual void			SetExternalInterfaceRetVal(const GFxValue& a_val);	// 32 - pure
	virtual void*			GetUserData() const;	// 33 - pure
	virtual void			SetUserData(void* a_data);	// 34 - pure
	virtual bool			AttachDisplayCallback(const char* a_pathToObject, void(*a_callback)(void* a_user), void* a_user);	// 35 - pure
	virtual bool			IsMovieFocused() const;	// 36 - pure
	virtual bool			GetDirtyFlag(bool a_doReset = true);	// 37 - pure
	virtual void			SetMouseCursorCount(UInt32 a_count);	// 38 - pure
	virtual UInt32			GetMouseCursorCount() const;	// 39 - pure
	virtual void			SetControllerCount(UInt32 a_count);	// 3A - pure
	virtual UInt32			GetControllerCount() const;	// 3B - pure
	virtual void			GetStats(GStatBag* a_bag, bool a_reset = true);	// 3C - pure
	virtual void*			GetHeap() const;	// 3D - pure
	virtual void			ForceCollectGarbage();	// 3E - pure
	virtual GPointF			TranslateToScreen(const GPointF& a_p, void* a_userMatrix);	// 3F - pure
	virtual GRectF			TranslateToScreen(const GRectF& a_p, void* a_userMatrix);	// 40 - pure
	virtual bool			TranslateLocalToScreen(const char* a_pathToCharacter, const GPointF& a_pt, GPointF* a_presPt, void* a_userMatrix);	// 41 - pure
	virtual bool			SetControllerFocusGroup(UInt32 a_controllerIdx, UInt32 a_focusGroupIndex);	// 42 - pure
	virtual UInt32			GetControllerFocusGroup(UInt32 a_controllerIdx) const;	// 43 - pure
	virtual void*			GetMemoryContext() const;	// 44 - pure
	virtual void			Release();
	
protected:
	void*	stateBag;	// 10
};

class GFxMovieRoot : public GFxMovieView
{
	enum class Flag : UInt32
	{
		kNone = 0,
		kDirty = 1 << 10,
		kMovieFocused = 1 << 18,
		kPaused = 1 << 20
	};


	virtual ~GFxMovieRoot();

	// add
	virtual void						Unk_46(void);																																									// 46
	virtual void						Unk_47(void);																																									// 47
	virtual void						Unk_48(void);																																									// 48


	// members
	UInt64				unk0018;							// 0018
	UInt64				unk0020;							// 0020
	void*				refCountCollector;					// 0028
	UInt64				unk0030;							// 0030
	GMemoryHeap*		heap;								// 0038
	UInt64				unk0040;							// 0040
	UInt64				unk0048;							// 0048
	UInt64				unk0050;							// 0050
	void*				unk0058;							// 0058
	GFxMovieDef*		movieDef;							// 0060
	UInt64				unk0068;							// 0068
	UInt64				unk0070;							// 0070
	GViewport			unk0078;							// 0078
	UInt64				unk00B0;							// 00B0
	UInt64				unk00B8;							// 00B8
	UInt32				viewScaleMode;						// 00C0
	UInt32				viewAlignment;						// 00C4
	GRectF				visibleFrameRect;					// 00C8
	UInt64				unk00D8;							// 00D8
	GRectF				safeRect;							// 00E0
	UInt64				unk00F0;							// 00F0
	UInt64				unk00F8;							// 00F8
	GMatrix3D*			perspective3D;						// 0100
	UInt64				unk0108;							// 0108
	UInt64				unk0110;							// 0110
	UInt64				unk0118[(0x09A0 - 0x0118) >> 3];	// 0118
	GColor				backgroundColor;					// 09A0
	UInt32				unk09A4;							// 09A4
	UInt64				unk09A8[(0x0A68 - 0x09A8) >> 3];	// 09A8
	UInt32				mouseCursorCount;					// 0A68
	UInt32				controllerCount;					// 0A6C
	void*				userData;							// 0A70
	UInt64				unk0A78;							// 0A78
	GFxKeyboardState	keyboardState;						// 0A80
	UInt64				unk1108[(0x25E0 - 0x1108) >> 3];	// 1108
	Flag				flags;								// 25E0
	UInt32				unk25E4;							// 25E4
	UInt64				unk25E8[(0x2B48 - 0x25E8) >> 3];	// 25E8
	UInt32				focusGroup;							// 2B48
	UInt8				controllerGroups[16];				// 2B4C
	UInt32				unk2B54;							// 2B54
	UInt64				unk2B58[(0x2BE8 - 0x2B58) >> 3];	// 2B58
};
STATIC_ASSERT(sizeof(GFxMovieRoot) == 0x2BF0);