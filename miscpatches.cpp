#include <intrin.h>

//#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Trampoline.h"

#include "patches.h"
#include "utils.h"

namespace patches
{
    decltype(&fopen_s) VC140_fopen_s;
    errno_t hk_fopen_s(FILE** File, const char* Filename, const char* Mode)
    {
        errno_t err = VC140_fopen_s(File, Filename, Mode);

        if (err != 0)
            _MESSAGE("WARNING: Error occurred trying to open file: fopen_s(%s, %s), errno %d", Filename, Mode, err);

        return err;
    }

    decltype(&_wfopen_s) VC140_wfopen_s;
    errno_t hk_wfopen_s(FILE** File, const wchar_t* Filename, const wchar_t* Mode)
    {
        errno_t err = VC140_wfopen_s(File, Filename, Mode);

        if (err != 0)
            _MESSAGE("WARNING: Error occurred trying to open file: _wfopen_s(%p, %p), errno %d", Filename, Mode, err);

        return err;
    }

    decltype(&fopen) VC140_fopen;
    FILE* hk_fopen(const char* Filename, const char* Mode)
    {
        FILE* f = VC140_fopen(Filename, Mode);

        if (!f)
            _MESSAGE("WARNING: Error occurred trying to open file: fopen(%s, %s)", Filename, Mode);

        return f;
    }

    bool PatchMaxStdio()
    {
        _VMESSAGE("- max stdio -");

        const HMODULE crtStdioModule = GetModuleHandleA("API-MS-WIN-CRT-STDIO-L1-1-0.DLL");

        if (!crtStdioModule)
        {
            _VMESSAGE("crt stdio module not found, failed");
            return false;
        }

        const auto maxStdio = reinterpret_cast<decltype(&_setmaxstdio)>(GetProcAddress(crtStdioModule, "_setmaxstdio"))(2048);

        _VMESSAGE("max stdio set to %d", maxStdio);

        *(void**)&VC140_fopen_s = (uintptr_t*)PatchIAT(unrestricted_cast<std::uintptr_t>(hk_fopen_s), "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen_s");
        *(void**)&VC140_wfopen_s = (uintptr_t*)PatchIAT(unrestricted_cast<std::uintptr_t>(hk_wfopen_s), "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "wfopen_s");
        *(void**)&VC140_fopen = (uintptr_t*)PatchIAT(unrestricted_cast<std::uintptr_t>(hk_fopen), "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen");

        return true;
    }

 //   REL::Offset<std::uintptr_t> AchievementModsEnabledFunction(AchievementModsEnabledFunction_offset);

    std::uintptr_t AchievemetModsEnabled = REL::Module::BaseAddr() + AchievementModsEnabledFunction_offset;

    bool PatchEnableAchievementsWithMods()
    {
        _VMESSAGE("- enable achievements with mods -");
        // Xbyak is used here to generate the ASM to use instead of just doing it by hand
        struct Patch : SKSE::CodeGenerator
        {
            Patch() : SKSE::CodeGenerator(100)
            {
                mov(al, 0);
                ret();
            }
        };

        Patch patch;
        patch.finalize();

        for (UInt32 i = 0; i < patch.getSize(); ++i)
        {
            SKSE::SafeWrite8(AchievemetModsEnabled + i, patch.getCode()[i]);
        }

        _VMESSAGE("success");
        return true;
    }

    REL::Offset<std::uintptr_t> SleepWaitTime_Compare = (0x8ea840 + 0x193);     // 8ea840 VR for top of function --- 0x193 offset to COMISS XMM0, dword ptr[DAT_14159a628]

    bool PatchSleepWaitTime()
    {
        _VMESSAGE("- sleep wait time -");
        {
            struct SleepWaitTime_Code : SKSE::CodeGenerator
            {
                SleepWaitTime_Code() : SKSE::CodeGenerator()
                {
                    push(rax);
                    mov(rax, (size_t)&config::sleepWaitTimeModifier);
                    comiss(xmm0, ptr[rax]);
                    pop(rax);
                    jmp(ptr[rip]);
                    dq(SleepWaitTime_Compare.GetAddress() + 0x7);
                }
            };

            SleepWaitTime_Code code;
            code.ready();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(SleepWaitTime_Compare.GetAddress(), code.getCode());
        }
        _VMESSAGE("success");
        return true;
    }




}
