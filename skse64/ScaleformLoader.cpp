#include "ScaleformLoader.h"
#include "Translation.h"

#include "skse64_common/Relocation.h"



GFxLoader * GFxLoader::GetSingleton()
{
	// 288124718F8102AA5D2B09022062AF83F3795442+AE
	RelocPtr<GFxLoader*> g_GFxLoader(0x02FEA518);
	return *g_GFxLoader;	
}

