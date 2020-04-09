#include "ObScript.h"
#include "skse64_common\Relocation.h"

// 698845F80EE915EE803E515A37C1269ED6A2FE43+47
RelocPtr <ObScriptCommand>	g_firstObScriptCommand(0x01E75A00);
// 698845F80EE915EE803E515A37C1269ED6A2FE43+1E
RelocPtr <ObScriptCommand>	g_firstConsoleCommand(0x01E83FB0);

RelocAddr<_ObScript_ExtractArgs> ObjScript_ExtractArgs(0x002F9900);