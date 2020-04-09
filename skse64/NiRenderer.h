#pragma once

#include "skse64/NiObjects.h"
#include "skse64/NiTextures.h"

#include "skse64/GameTypes.h"

struct ID3D11Device;
struct ID3D11Forwarder;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

// Unknown class name, No RTTI
class BSRenderManager
{
public:
	static BSRenderManager * GetSingleton();

	UInt64						unk00[0x50 >> 3];	// 00
	ID3D11Device				* forwarder;		// 50 - Actually CID3D11Forwarder
	ID3D11DeviceContext			* context;			// 58 - ID3D11DeviceContext4
	UInt64						unk58;				// 60
	UInt64						unk60;				// 68
	UInt64						unk68;				// 70
	IDXGISwapChain				* swapChain;		// 78
	UInt64						unk78;				// 80
	UInt64						unk80;				// 88
	ID3D11RenderTargetView		* renderView;		// 90
	ID3D11ShaderResourceView	* resourceView;		// 98
	UInt64						unkA0[(0x2F00 - 0xA0) >> 3];
	CRITICAL_SECTION			lock;				// 2F00

	// 1412F0390 - CreateRenderTargets

	MEMBER_FN_PREFIX(BSRenderManager);
	DEFINE_MEMBER_FN(CreateRenderTexture, NiTexture::RendererData *, 0x00DBF9B0, UInt32 width, UInt32 height);
};
STATIC_ASSERT(offsetof(BSRenderManager, lock) == 0x2F00);

extern RelocPtr <BSRenderManager> g_renderManager;

class BSShaderResourceManager
{
public:
	virtual ~BSShaderResourceManager();
};
extern RelocPtr <BSShaderResourceManager> g_shaderResourceManager;
