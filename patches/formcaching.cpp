#include <type_traits>

#include "tbb/concurrent_hash_map.h"

#include "RE/Skyrim.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/Trampoline.h"

#include "patches.h"

namespace patches
{
    // treelodreferencecaching.cpp
    void InvalidateCachedForm(uint32_t formId);

    std::uintptr_t base = REL::Module::BaseAddr();

    tbb::concurrent_hash_map<uint32_t, RE::TESForm*> globalFormCacheMap[256];

    REL::Offset<RE::BSReadWriteLock*> GlobalFormTableLock(0x1f88fb0);     // 1ec4150
    REL::Offset<RE::BSTHashMap<UInt32, RE::TESForm*>**> GlobalFormTable(0x1f88b18); // 1ec3cb8   delta of ‬‬0xc4e60

    typedef void (*UnknownFormFunction0_)(__int64 form, bool a2);
    REL::Offset<UnknownFormFunction0_> origFunc0HookAddr(0x1a4520);    // 1947f0  delta of 0xfd30
    UnknownFormFunction0_ origFunc0;

    typedef __int64 (*UnknownFormFunction1_)(__int64 a1, __int64 a2, int a3, DWORD* formId, __int64* a5);
    REL::Offset<UnknownFormFunction1_> origFunc1HookAddr(0x1a5c20);    // 195ef0
    UnknownFormFunction1_ origFunc1;

    typedef __int64 (*UnknownFormFunction2_)(__int64 a1, __int64 a2, int a3, DWORD* formId, __int64** a5);
    REL::Offset<UnknownFormFunction2_> origFunc2HookAddr(0x1a5950); // 195c20
    UnknownFormFunction2_ origFunc2;

    typedef __int64 (*UnknownFormFunction3_)(__int64 a1, __int64 a2, int a3, __int64 a4);
    REL::Offset<UnknownFormFunction3_> origFunc3HookAddr(0x1a6510);    // 1967e0
    UnknownFormFunction3_ origFunc3;

    void UpdateFormCache(uint32_t FormId, RE::TESForm* Value, bool Invalidate)
    {
        const unsigned char masterId = (FormId & 0xFF000000) >> 24;
        const unsigned int baseId = (FormId & 0x00FFFFFF);

        if (Invalidate)
            globalFormCacheMap[masterId].erase(baseId);
        else
            globalFormCacheMap[masterId].insert(std::make_pair(baseId, Value));

        InvalidateCachedForm(FormId);
    }

    RE::TESForm* hk_GetFormByID(unsigned int FormId)
    {
        RE::TESForm* formPointer = nullptr;

        const unsigned char masterId = (FormId & 0xFF000000) >> 24;
        const unsigned int baseId = (FormId & 0x00FFFFFF);

        {
            std::remove_extent_t<decltype(globalFormCacheMap)>::accessor accessor;

            if (globalFormCacheMap[masterId].find(accessor, baseId))
            {
                formPointer = accessor->second;
                return formPointer;
            }
        }

        // Try to use Bethesda's scatter table which is considerably slower
        GlobalFormTableLock->LockForRead();

        if (*GlobalFormTable)
        {
            auto iter = (*GlobalFormTable)->find(FormId);
            formPointer = (iter != (*GlobalFormTable)->end()) ? iter->second : nullptr;
    ///        _MESSAGE("FormID = %x -> %016I64X", FormId, formPointer);
        }

        GlobalFormTableLock->UnlockForRead();

        if (formPointer)
            UpdateFormCache(FormId, formPointer, false);

        return formPointer;
    }

    __int64 UnknownFormFunction3(__int64 a1, __int64 a2, int a3, __int64 a4)
    {
        UpdateFormCache(*(uint32_t*)a4, nullptr, true);

        return origFunc3(a1, a2, a3, a4);
    }

    __int64 UnknownFormFunction2(__int64 a1, __int64 a2, int a3, DWORD* formId, __int64** a5)
    {
        UpdateFormCache(*formId, nullptr, true);

        return origFunc2(a1, a2, a3, formId, a5);
    }

    __int64 UnknownFormFunction1(__int64 a1, __int64 a2, int a3, DWORD* formId, __int64* a5)
    {
        UpdateFormCache(*formId, nullptr, true);

        return origFunc1(a1, a2, a3, formId, a5);
    }

    void UnknownFormFunction0(__int64 form, bool a2)
    {
        UpdateFormCache(*(uint32_t*)(form + 0x14), nullptr, true);

        origFunc0(form, a2);
    }

    bool PatchFormCaching()
    {
        _MESSAGE("- form caching -");

        REL::Offset<std::uint32_t*> LookupFormByID(0x1a3f60);     // SSE has this offset as 0x194230
//        uintptr_t LookupFormByID = REL::Module::BaseAddr() + (uintptr_t)0x194230;
        _VMESSAGE("gimmie addr %016I64X", LookupFormByID.GetAddress());
        _VMESSAGE("%016I64X", *LookupFormByID);

        if (*LookupFormByID != 0x83485740)
        {
            _VMESSAGE("sse fixes is installed and enabled. aborting form cache patch.");
            config::patchFormCaching = false;

            return false;
        }

        _VMESSAGE("detouring GetFormById");
        _VMESSAGE("gimmie addr %016I64X", unrestricted_cast<std::uintptr_t>(&hk_GetFormByID));

        SKSE::GetTrampoline()->Write6Branch(LookupFormByID.GetAddress(), unrestricted_cast<std::uintptr_t>(&hk_GetFormByID));
        _VMESSAGE("done");
 //       return true;
        // TODO: write a generic detour instead
        _VMESSAGE("detouring global form table write functions");
        {
            struct UnknownFormFunction0_Code : SKSE::CodeGenerator
            {
                UnknownFormFunction0_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;

                    // 194970
                    mov(r11, rsp);
                    push(rbp);
                    push(rsi);
                    push(rdi);
                    push(r12);
                    // 194978

                    // exit
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc0HookAddr.GetAddress() + 0x8);
                }
            };

            UnknownFormFunction0_Code code;
            code.finalize();
            origFunc0 = UnknownFormFunction0_(code.getCode());

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(origFunc0HookAddr.GetAddress(), unrestricted_cast<std::uintptr_t>(&UnknownFormFunction0));
        }

        {
            struct UnknownFormFunction1_Code : SKSE::CodeGenerator
            {
                UnknownFormFunction1_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;

                    // 196070
                    push(rdi);
                    push(r14);
                    push(r15);
                    sub(rsp, 0x20);
                    // 19607A

                    // exit
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc1HookAddr.GetAddress() + 0xA);
                }
            };

            UnknownFormFunction1_Code code;
            code.finalize();
            origFunc1 = UnknownFormFunction1_(code.getCode());

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(origFunc1HookAddr.GetAddress(), unrestricted_cast<std::uintptr_t>(&UnknownFormFunction1));
        }

        {
            struct UnknownFormFunction2_Code : SKSE::CodeGenerator
            {
                UnknownFormFunction2_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;

                    // 195DA0
                    mov(ptr[rsp + 0x10], rbx);
                    push(rsi);
                    sub(rsp, 0x20);
                    // 195DAA

                    // exit
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc2HookAddr.GetAddress() + 0xA);
                }
            };

            UnknownFormFunction2_Code code;
            code.finalize();
            origFunc2 = UnknownFormFunction2_(code.getCode());

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(origFunc2HookAddr.GetAddress(), unrestricted_cast<std::uintptr_t>(&UnknownFormFunction2));
        }

        {
            struct UnknownFormFunction3_Code : SKSE::CodeGenerator
            {
                UnknownFormFunction3_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;

                    // 196960
                    push(rbp);
                    push(rsi);
                    push(r14);
                    sub(rsp, 0x20);
                    // 196969

                    // exit
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc3HookAddr.GetAddress() + 0x9);
                }
            };

            UnknownFormFunction3_Code code;
            code.finalize();
            origFunc3 = UnknownFormFunction3_(code.getCode());

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(origFunc3HookAddr.GetAddress(), unrestricted_cast<std::uintptr_t>(&UnknownFormFunction3));
        }
        _VMESSAGE("done");

        _MESSAGE("success");

        return true;
    }
}
