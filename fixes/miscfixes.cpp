#include <array>
#include <map>
#include <utility>
#include <sysinfoapi.h>
#include <algorithm>

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Trampoline.h"

#include "oneapi/tbb/concurrent_hash_map.h"

#include "fixes.h"
#include "utils.h"

namespace fixes
{

    class MemoryAccessErrorsPatch
    {
    public:
        static void Install()
        {
            PatchSnowMaterialCase();
            PatchShaderParticleGeometryDataLimit();
            PatchUseAfterFree();
        }

    private:
        static void PatchSnowMaterialCase()
        {
            _VMESSAGE("patching BSLightingShader::SetupMaterial snow material case");

            struct Patch : SKSE::CodeGenerator
            {
                Patch(std::uintptr_t a_vtbl, std::uintptr_t a_hook, std::uintptr_t a_exit) : SKSE::CodeGenerator()
                {
                    Xbyak::Label vtblAddr;
                    Xbyak::Label snowRetnLabel;
                    Xbyak::Label exitRetnLabel;

                    mov(rax, ptr[rip + vtblAddr]);
                    cmp(rax, qword[rbx]);
                    je("IS_SNOW");

                    // not snow, fill with 0 to disable effect
                    mov(eax, 0x00000000);
                    mov(dword[rcx + rdx * 4 + 0xC], eax);
                    mov(dword[rcx + rdx * 4 + 0x8], eax);
                    mov(dword[rcx + rdx * 4 + 0x4], eax);
                    mov(dword[rcx + rdx * 4], eax);
                    jmp(ptr[rip + exitRetnLabel]);

                    // is snow, jump out to original except our overwritten instruction
                    L("IS_SNOW");
                    movss(xmm2, dword[rbx + 0xAC]);
                    jmp(ptr[rip + snowRetnLabel]);

                    L(vtblAddr);
                    dq(a_vtbl);

                    L(snowRetnLabel);
                    dq(a_hook + 0x8);

                    L(exitRetnLabel);
                    dq(a_exit);
                }
            };

            REL::Offset<std::uintptr_t> vtbl = 0x18fea78;    // SE is 12d38f3   VR is 18fea78
            REL::Offset<std::uintptr_t> funcBase = 0x1338400;   // SE is 12f2020  VR is 1338400
            std::uintptr_t hook = funcBase.GetAddress() + 0x4E0;
            std::uintptr_t exit = funcBase.GetAddress() + 0x5B6;
            vtbl.GetAddress();
            _VMESSAGE("vtbl     = %016I64X", vtbl.GetAddress());
            _VMESSAGE("funcBase = %016I64X", funcBase.GetAddress());


            Patch patch(vtbl.GetAddress(), hook, exit);
            patch.ready();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(hook, patch.getCode<std::uintptr_t>());
        }

        static void PatchShaderParticleGeometryDataLimit()
        {
            _VMESSAGE("patching BGSShaderParticleGeometryData limit");

            REL::Offset<std::uintptr_t> vtbl = 0x15ebf50;
            _Load = vtbl.WriteVFunc(0x6, Load);
        }

        static void PatchUseAfterFree()
        {
            _VMESSAGE("patching BSShadowDirectionalLight use after free");

            // Xbyak is used here to generate the ASM to use instead of just doing it by hand
            struct Patch : SKSE::CodeGenerator
            {
                Patch() : SKSE::CodeGenerator(100)
                {
                    mov(r9, r15);
                    nop();
                    nop();
                    nop();
                    nop();
                }
            };

            Patch patch;
            patch.ready();

            REL::Offset<std::uintptr_t> target = 0x13574f0;  // SE is 13251c0   VR is 13574f0

            for (std::size_t i = 0; i < patch.getSize(); ++i)
            {
                SKSE::SafeWrite8(target.GetAddress() + 0x1c6d + i, patch.getCode()[i]);
            }
        }

        // BGSShaderParticleGeometryData::Load
        static bool Load(RE::BGSShaderParticleGeometryData* a_this, RE::TESFile* a_file)
        {
            const bool retVal = _Load(a_this, a_file);

            // the game doesn't allow more than 10 here
            if (a_this->data.size() >= 12)
            {
                const auto particleDensity = a_this->data[11];
                if (particleDensity.f > 10.0)
                    a_this->data[11].f = 10.0f;
            }

            return retVal;
        }

        inline static REL::Function<decltype(&RE::BGSShaderParticleGeometryData::Load)> _Load;
    };

    bool PatchMemoryAccessErrors()
    {
        _VMESSAGE("- memory access errors fix -");

         MemoryAccessErrorsPatch::Install();

        _VMESSAGE("success");
        return true;
    }



    class LipSyncPatch
    {
    public:
        static void Install()
        {
            constexpr std::array<UInt8, 4> OFFSETS = {
                0x1E,
                0x3A,
                0x9A,
                0xD8
            };
            
            REL::Offset<std::uintptr_t> funcBase = 0x201d80;   // SE is 1f12a0  VR is 201d80
            for (auto& offset : OFFSETS)
            {
                SKSE::SafeWrite8(funcBase.GetAddress() + offset, 0xEB);  // jns -> jmp
            }
        }
    };

    bool PatchLipSync()
    {
        _VMESSAGE("- lip sync bug fix -");

        LipSyncPatch::Install();

        _VMESSAGE("- success -");
        return true;
    }

    class GHeapLeakDetectionCrashPatch
    {
    public:
        static void Install()
        {
            constexpr std::uintptr_t START = 0x4B;
            constexpr std::uintptr_t END = 0x5C;
            constexpr UInt8 NOP = 0x90;
            REL::Offset<std::uintptr_t> funcBase = 0x105cbb0;   //SE is fffeb0  VR is 105cbb0

            for (std::uintptr_t i = START; i < END; ++i)
            {
                SKSE::SafeWrite8(funcBase.GetAddress() + i, NOP);
            }
        }
    };

    bool PatchGHeapLeakDetectionCrash()
    {
        _VMESSAGE("- GHeap leak detection crash fix -");

        GHeapLeakDetectionCrashPatch::Install();

        _VMESSAGE("success");
        return true;
    }


    class CellInitPatch
    {
    public:
        static void Install()
        {
            auto trampoline = SKSE::GetTrampoline();
            REL::Offset<std::uintptr_t> funcBase = 0x273830;  // SE is 262290  VR is 273830
            _GetLocation = trampoline->Write5CallEx(funcBase.GetAddress() + 0x110, GetLocation);
        }

    private:
        static RE::BGSLocation* GetLocation(const RE::ExtraDataList* a_this)
        {
            auto cell = adjust_pointer<RE::TESObjectCELL>(a_this, -0x48);
            auto loc = _GetLocation(a_this);
            if (!cell->IsInitialized())
            {
                auto file = cell->GetFile();
                auto formID = reinterpret_cast<RE::FormID>(loc);
                RE::TESForm::AddCompileIndex(formID, file);
                loc = RE::TESForm::LookupByID<RE::BGSLocation>(formID);
            }
            return loc;
        }

        static inline REL::Function<decltype(GetLocation)> _GetLocation;
    };

    bool PatchCellInit()
    {
        _VMESSAGE("- cell init fix -");

        CellInitPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    class AnimationLoadSignedCrashPatch
    {
    public:
        static void Install()
        {
            // Change "BF" to "B7"
            REL::Offset<std::uintptr_t> target =  0xba17a0 + 0x91;   // SE is b66930   VR is ba17a0
            SKSE::SafeWrite8(target.GetAddress(), 0xB7);
        }
    };

    bool PatchAnimationLoadSignedCrash()
    {
        _VMESSAGE("- animation load signed crash fix -");

        AnimationLoadSignedCrashPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    class BSLightingAmbientSpecularPatch
    {
    public:
        static void Install()
        {
            _VMESSAGE("nopping SetupMaterial case");

            constexpr byte nop = 0x90;
            constexpr uint8_t length = 0x20;

            REL::Offset<std::uintptr_t> addAmbientSpecularToSetupGeometry = 0x1339bb9;   // SE is 12f2bb0  VR is 1338f60   0xBAD in SE goes to 1339bb9 which is offset 0xC59
            REL::Offset<std::uintptr_t> ambientSpecularAndFresnel = 0x342317c;           // SE is 1e0dfcc  VR is 342317c
            REL::Offset<std::uintptr_t> disableSetupMaterialAmbientSpecular = 0x1338400 + 0x713;               // SE is 12f2020  VR is 1338400

            for (int i = 0; i < length; ++i)
            {
                SKSE::SafeWrite8(disableSetupMaterialAmbientSpecular.GetAddress() + i, nop);
            }

            _VMESSAGE("Adding SetupGeometry case");

            struct Patch : SKSE::CodeGenerator
            {
                Patch(std::uintptr_t a_ambientSpecularAndFresnel, std::uintptr_t a_addAmbientSpecularToSetupGeometry) : SKSE::CodeGenerator()
                {
                    Xbyak::Label jmpOut;
                    // hook: 0x130AB2D (in middle of SetupGeometry, right before if (rawTechnique & RAW_FLAG_SPECULAR), just picked a random place tbh
                    // test
                    test(dword[rbx + 0x94], 0x20000);  // RawTechnique & RAW_FLAG_AMBIENT_SPECULAR   :   VR is using RBX not R13
                    jz(jmpOut);
                    // ambient specular
                    push(rax);
                    push(rdx);
                    mov(rax, a_ambientSpecularAndFresnel);  // xmmword_1E3403C
                    movups(xmm0, ptr[rax]);
                    mov(rax, qword[rsp + 0x170 - 0x120 + 0x8]);  // PixelShader  // was 0x10 instead of 0x8.  Seems pixelshader is now rsp+0x58 but not entirely sure.   Looks right in reclass
                    movzx(edx, byte[rax + 0x46]);                 // m_ConstantOffsets 0x6 (AmbientSpecularTintAndFresnelPower)
                    mov(rax, ptr[rdi + 8]);                       // m_PerGeometry buffer (copied from SetupGeometry)   // VR is using RDI not R15
                    movups(ptr[rax + rdx * 4], xmm0);             // m_PerGeometry buffer offset 0x6
                    pop(rdx);
                    pop(rax);
                    // original code
                    L(jmpOut);
                    test(dword[rbx + 0x94], 0x200);
                    jmp(ptr[rip]);
                    dq(a_addAmbientSpecularToSetupGeometry + 11);
                }
            };

            Patch patch(ambientSpecularAndFresnel.GetAddress(), addAmbientSpecularToSetupGeometry.GetAddress());
            patch.ready();

            _VMESSAGE("specularandfresnel = %016I64X", addAmbientSpecularToSetupGeometry.GetAddress());
            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write5Branch(addAmbientSpecularToSetupGeometry.GetAddress(), reinterpret_cast<std::uintptr_t>(patch.getCode()));
        }
    };

    bool PatchBSLightingAmbientSpecular()
    {
        _VMESSAGE("BSLightingAmbientSpecular fix");

        BSLightingAmbientSpecularPatch::Install();

        _VMESSAGE("success");
        return true;
    }


    class CalendarSkippingPatch
    {
    public:
        static void Install()
        {
            constexpr std::size_t CAVE_START = 0x17A;
            constexpr std::size_t CAVE_SIZE = 0x15;

            REL::Offset<std::uintptr_t> funcBase = 0x5ad8f0;  // SE is 5a6230  VR is 5ad8f0

            struct Patch : SKSE::CodeGenerator
            {
                Patch(std::uintptr_t a_addr) : SKSE::CodeGenerator(CAVE_SIZE)
                {
                    Xbyak::Label jmpLbl;

                    movaps(xmm0, xmm1);
                    jmp(ptr[rip + jmpLbl]);

                    L(jmpLbl);
                    dq(a_addr);
                }
            };

            Patch patch(unrestricted_cast<std::uintptr_t>(AdvanceTime));
            patch.ready();

            for (std::size_t i = 0; i < patch.getSize(); ++i)
            {
                SKSE::SafeWrite8(funcBase.GetAddress() + CAVE_START + i, patch.getCode()[i]);
            }
        }

    private:
        static void AdvanceTime(float a_secondsPassed)
        {
            auto time = RE::Calendar::GetSingleton();
            float hoursPassed = (a_secondsPassed * time->timeScale->value / (60.0F * 60.0F)) + time->gameHour->value - 24.0F;
            if (hoursPassed > 24.0)
            {
                do
                {
                    time->midnightsPassed += 1;
                    time->rawDaysPassed += 1.0F;
                    hoursPassed -= 24.0F;
                } while (hoursPassed > 24.0F);
                time->gameDaysPassed->value = (hoursPassed / 24.0F) + time->rawDaysPassed;
            }
        }
    };

    bool PatchCalendarSkipping()
    {
        _VMESSAGE("- calendar skipping fix -");

        CalendarSkippingPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    class ConjurationEnchantAbsorbsPatch
    {
    public:
        static void Install()
        {
            REL::Offset<std::uintptr_t> vtbl = 0x1598b30;     // SE is 15217e0   VR is 1598b30
            _DisallowsAbsorbReflection = vtbl.WriteVFunc(0x5E, DisallowsAbsorbReflection);
        }

    private:
        static bool DisallowsAbsorbReflection(RE::EnchantmentItem* a_this)
        {
            using Archetype = RE::EffectArchetypes::ArchetypeID;
            for (auto& effect : a_this->effects)
            {
                if (effect->baseEffect->HasArchetype(Archetype::kSummonCreature))
                {
                    return true;
                }
            }
            return _DisallowsAbsorbReflection(a_this);
        }

        using DisallowsAbsorbReflection_t = decltype(&RE::EnchantmentItem::GetNoAbsorb);  // 5E
        static inline REL::Function<DisallowsAbsorbReflection_t> _DisallowsAbsorbReflection;
    };

    bool PatchConjurationEnchantAbsorbs()
    {
        _VMESSAGE("- enchantment absorption on staff summons fix -");

        ConjurationEnchantAbsorbsPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    class EquipShoutEventSpamPatch
    {
    public:
        static void Install()
        {
            constexpr std::uintptr_t BRANCH_OFF = 0x17A;
            constexpr std::uintptr_t SEND_EVENT_BEGIN = 0x18A;
            constexpr std::uintptr_t SEND_EVENT_END = 0x236;
            constexpr std::size_t EQUIPPED_SHOUT = offsetof(RE::Actor, selectedPower);
            constexpr UInt32 BRANCH_SIZE = 5;
            constexpr UInt32 CODE_CAVE_SIZE = 16;
            constexpr UInt32 DIFF = CODE_CAVE_SIZE - BRANCH_SIZE;
            constexpr UInt8 NOP = 0x90;

            REL::Offset<std::uintptr_t> funcBase = 0x63b290;  // SE is 6323c0 VR is 63b290

            struct Patch : SKSE::CodeGenerator
            {
                Patch(std::uintptr_t a_funcBase) : SKSE::CodeGenerator()
                {
                    Xbyak::Label exitLbl;
                    Xbyak::Label exitIP;
                    Xbyak::Label sendEvent;

                    // r14 = Actor*
                    // rdi = TESShout*

                    cmp(ptr[r14 + EQUIPPED_SHOUT], rdi);  // if (actor->equippedShout != shout)
                    je(exitLbl);
                    mov(ptr[r14 + EQUIPPED_SHOUT], rdi);  // actor->equippedShout = shout;
                    test(rdi, rdi);                       // if (shout)
                    jz(exitLbl);
                    jmp(ptr[rip + sendEvent]);

                    L(exitLbl);
                    jmp(ptr[rip + exitIP]);

                    L(exitIP);
                    dq(a_funcBase + SEND_EVENT_END);

                    L(sendEvent);
                    dq(a_funcBase + SEND_EVENT_BEGIN);
                }
            };

            Patch patch(funcBase.GetAddress());
            patch.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write5Branch(funcBase.GetAddress() + BRANCH_OFF, reinterpret_cast<std::uintptr_t>(patch.getCode()));

            for (UInt32 i = 0; i < DIFF; ++i)
            {
                SKSE::SafeWrite8(funcBase.GetAddress() + BRANCH_OFF + BRANCH_SIZE + i, NOP);
            }
        }
    };

    bool PatchEquipShoutEventSpam()
    {
        _VMESSAGE("- equip shout event spam fix - ");

        EquipShoutEventSpamPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    class PerkFragmentIsRunningPatch
    {
    public:
        static void Install()
        {
            REL::Offset<std::uintptr_t> funcBase = 0x2ecb20;  // SE is 2db610 VR is 2ecb20
            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write5Call(funcBase.GetAddress() + 0x22, IsRunning);
        }

    private:
        static bool IsRunning(RE::Actor* a_this)
        {
            return a_this ? a_this->IsRunning() : false;
        }
    };

    bool PatchPerkFragmentIsRunning()
    {
        _VMESSAGE("- perk fragment IsRunning fix -");

        PerkFragmentIsRunningPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    class RemovedSpellBookPatch
    {
    public:
        static void Install()
        {
            REL::Offset<std::uintptr_t> vtbl = 0x15c9e68;   // SE is 15592b8  VR is 15c9e68
            _LoadGame = vtbl.WriteVFunc(0xF, LoadGame);
        }

    private:
        static void LoadGame(RE::TESObjectBOOK* a_this, RE::BGSLoadFormBuffer* a_buf)
        {
            using Flag = RE::OBJ_BOOK::Flag;

            _LoadGame(a_this, a_buf);

            if (a_this->data.teaches.actorValueToAdvance == RE::ActorValue::kNone)
            {
                if (a_this->TeachesSkill())
                {
                    a_this->data.flags &= ~Flag::kAdvancesActorValue;
                }

                if (a_this->TeachesSpell())
                {
                    a_this->data.flags &= ~Flag::kTeachesSpell;
                }
            }
        }

        static inline REL::Function<decltype(&RE::TESObjectBOOK::LoadGame)> _LoadGame;  // 0xF
    };

    bool PatchRemovedSpellBook()
    {
        _VMESSAGE("- removed spell book fix -");

        RemovedSpellBookPatch::Install();

        _VMESSAGE("success");
        return true;
    }

    struct abilityTimerData {
        uint64_t lastTick;
        uint64_t updateTimer;
        uint64_t updateDiff;
        uint64_t lastCounter;
        bool updated = false;
    };

    class TimerData {
    public:
        long lastCounter = -1;
        long updateDiff = -1;
        long updateTimer = 0;
        long lastTick = 0;
        bool done = false;
    };

    tbb::concurrent_hash_map<uint64_t, TimerData*> timerData;

    class FixAbilityConditionBug
    {
    public:
        static void Install()
        {
            _VMESSAGE("nopping rest of if statement being replaced");

            constexpr byte nop = 0x90;
            constexpr uint8_t length = 0x79;

            REL::Offset<std::uintptr_t> jumpLoc = 0x54123d;
            REL::Offset<std::uintptr_t> returnFalse = 0x5412b6;   // MOV EAX,dword ptr [RDI+0x34]
            REL::Offset<std::uintptr_t> returnTrue = 0x54133d;    // MOV RBX,qword ptr [RSP + local_res10]

            for (int i = 0; i < length; ++i)
            {
                SKSE::SafeWrite8(jumpLoc.GetAddress() + i, nop);
            }

            struct Patch : SKSE::CodeGenerator
            {
                Patch(std::uintptr_t rf, std::uintptr_t rt) : SKSE::CodeGenerator()
                {
                    Xbyak::Label returnTrue;
                    Xbyak::Label returnFalse;
                    Xbyak::Label rf_addr;


                    mov(rax, (uintptr_t)newTimingFunc);
                    movss(xmm0, xmm6);
                    movss(xmm1, ptr[rdi + 0x70]);
                    mov(rcx, rdi);
                    
                    call(rax);     // Call new timing func

                    test(rax, rax);
                    jz(returnFalse);
                    //movss(xmm1, ptr[rdi + 0x70]);
                    jmp(ptr[rip + returnTrue]);

                    L(returnFalse);
                    jmp(ptr[rip+rf_addr]);

                    L(returnTrue);
                    dq(rt);

                    L(rf_addr);
                    dq(rf);
                }
            };

            Patch patch(returnFalse.GetAddress(), returnTrue.GetAddress());
            patch.finalize();

            _VMESSAGE("Installing Patch");

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write5Branch(jumpLoc.GetAddress(), reinterpret_cast<std::uintptr_t>(patch.getCode()));
        }

    private:

        static bool newTimingFunc(uint64_t rid_reg, float diff, float elapsedTime) {
            REL::Offset<std::uintptr_t> gameSettingValue = 0x1ea23e0;

            if (diff <= 0.0) {
         //       _VMESSAGE("diff less than 0");
                return true;
            }


            decltype(timerData)::accessor accessor;
            TimerData* td = nullptr;

            long now = (long)GetTickCount64();

            if (timerData.find(accessor, rid_reg)) {
                td = accessor->second;

                int diff = now - td->lastTick;
                if (diff == 0) {
                    return true;
                }

                td->lastTick = now;
                td->updateTimer += diff;

            }
            else {
                td = new TimerData;
                td->lastTick = now;

                timerData.insert(std::make_pair(rid_reg, td));
            }

            if (td->updateDiff < 0) {
                float* gsv = (float*)gameSettingValue.GetAddress();

                float interval = std::max(0.001f, (float)(*gsv));

                td->updateDiff = (long)(1000.0 / interval);

                if (td->updateDiff <= 0) {
                    td->updateDiff = 1;
                }
            }
//            _VMESSAGE("rid_reg %016I64X td->updateTimer %d", rid_reg, td->updateTimer);

            long cur = td->updateTimer / td->updateDiff;

            if (cur != td->lastCounter) {
                td->lastCounter = cur;
                return true;
            }
            return false;
        }

    };

    bool PatchFixAbilityConditionBug()
    {
        _VMESSAGE("PatchFixAbilityConditionBug fix");

        FixAbilityConditionBug::Install();

        _VMESSAGE("success");
        return true;
    }


    class FixBuySellStackSpeechGain
    {
    public:
        static void Install()
        {
            REL::Offset<std::uintptr_t> loc1be = 0x87ce5e;
            REL::Offset<std::uintptr_t> locd1  = 0x87cd71;
            REL::Offset<std::uintptr_t> funcCall = 0x1e7200;

            makePatches(loc1be.GetAddress(), locd1.GetAddress(), funcCall.GetAddress());   
        }

    private:
        struct Patch1 : SKSE::CodeGenerator
        {
            Patch1(std::uintptr_t addr_src, std::uintptr_t funcCall_addr) : SKSE::CodeGenerator() {
                Xbyak::Label exit1;

                mov(rax, funcCall_addr);
                call(rax);
                mov(rdx, ptr[rsp + 0xd0]);
                movss(xmm2, xmm0);
                mov(rax, (uintptr_t)multExp);
                call(rax);
                jmp(ptr[rip + exit1]);

                L(exit1);
                dq(addr_src + 5);
            }
        };

        struct Patch2 : SKSE::CodeGenerator
        {
            Patch2(std::uintptr_t addr_src, std::uintptr_t funcCall_addr) : SKSE::CodeGenerator() {
                Xbyak::Label exit1;

                mov(rax, funcCall_addr);
                call(rax);
                mov(rdx, ptr[rsp + 0xd0]);
                movss(xmm2, xmm0);
                mov(rax, (uintptr_t)multExp);
                call(rax);
                jmp(ptr[rip + exit1]);

                L(exit1);
                dq(addr_src + 5);
            }
        };

        static void makePatches(std::uintptr_t source_addr1, std::uintptr_t source_addr2, std::uintptr_t funcCall) {
            Patch1 patch1(source_addr1, funcCall);
            patch1.finalize();

            Patch2 patch2(source_addr2, funcCall);
            patch2.finalize();

            auto trampoline1 = SKSE::GetTrampoline();
            trampoline1->Write5Branch(source_addr1, reinterpret_cast<std::uintptr_t>(patch1.getCode()));

            auto trampoline2 = SKSE::GetTrampoline();
            trampoline2->Write5Branch(source_addr2, reinterpret_cast<std::uintptr_t>(patch2.getCode()));
        }

        static float multExp(float und, int count, float giveXp) {
            if (count > 1) {
                giveXp = giveXp * count;
            }

            return giveXp;
        }

    };

    bool PatchFixBuySellStackSpeechGain()
    {
        _VMESSAGE("PatchFixBuySellStackSpeechGain fix");

        FixBuySellStackSpeechGain::Install();

        _VMESSAGE("success");
        return true;
    }

    bool PatchShadowSceneCrash() {
        _MESSAGE("--- Patching ShadowSceneNode function common crash");

        REL::Offset<std::uintptr_t> jumpLoc    = 0x12f86dd;
        REL::Offset<std::uintptr_t> retFunc    = 0x12f8780;
        REL::Offset<std::uintptr_t> callLoc    = 0x12f86e6;

        for (int i = 0; i < 9; ++i)
        {
            SKSE::SafeWrite8(jumpLoc.GetAddress() + i, 0x90);
        }


        struct Patch : SKSE::CodeGenerator
        {
            Patch(std::uintptr_t rf, std::uintptr_t rt) : SKSE::CodeGenerator()
            {
                Xbyak::Label returnTrue;
                Xbyak::Label returnFalse;
                Xbyak::Label retAddr;

                mov(rdi, rcx);
                mov(rcx, rdx);
                mov(rbx, rdx);
                mov(r8, ptr[rax + 0x18]);

                test(r8, r8);
                jz(returnTrue);
                jmp(ptr[rip+returnFalse]);

                L(returnFalse);
                dq(rf);

                L(returnTrue);
                jmp(ptr[rip+retAddr]);

                L(retAddr);
                dq(rt);

            }
        };

        Patch patch(callLoc.GetAddress(), retFunc.GetAddress());
        patch.finalize();

        _VMESSAGE("Installing Patch");

        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write5Branch(jumpLoc.GetAddress(), reinterpret_cast<std::uintptr_t>(patch.getCode()));

        return true;
    }

    bool PatchShadowSceneNodeNullptrCrash()
    {
        // from https://github.com/aers/EngineFixesSkyrim64/blob/master/src/fixes/miscfixes.cpp#L1342-L1405
        // written for 1.5.97 but should be compatible and adjusted for VR

        // sub_1412BACA0 in 1.5.97, 0x12f86d0 in VR
        REL::Offset<std::uintptr_t> baseFuncAddr = 0x12f86d0;
        const uint64_t faddr = baseFuncAddr.GetAddress(); //offsets::ShadowSceneNodeNullPtr::FuncBase.address();
        _MESSAGE("- workaround for crash in ShadowSceneNode::unk_%016I64X -", faddr);
        const uint8_t* crashaddr = (uint8_t*)(uintptr_t)(faddr + 0x16);
        /*
                        mov     rax,qword ptr [rdx]
        ... some movs ...
        FF 50 18        call    qword ptr [rax+18h] // <--- crashes here because rax == 0
        84 C0           test    al, al
        74 ??           jz      short loc_LAB_1412f8703
        */

        static const uint8_t expected[] = { 0xFF, 0x50, 0x18, 0x84, 0xC0, 0x74 };
        if (std::memcmp((const void*)crashaddr, expected, sizeof(expected)))
        {
            _MESSAGE("Code is not as expected, skipping patch");
            return false;
        }

        const int8_t disp8 = crashaddr[sizeof(expected)]; // jz short displacement (should be 0x16)
        const uint8_t* jmpdstZero = crashaddr + sizeof(expected) + disp8 + 1;
        const uint8_t* jmpdstNonZero = crashaddr + sizeof(expected) + 1;

        struct Code : SKSE::CodeGenerator
        {
            Code(std::uintptr_t contNonzeroAddr, std::uintptr_t contZeroAddr) : SKSE::CodeGenerator()
            {
                Xbyak::Label contAddrLbl, zeroLbl, zeroAddrLbl;

                // check for NULL
                test(rax, rax);
                jz(zeroLbl);

                // original instructions minus jz short
                db(expected, sizeof(expected) - 1);

                jz(zeroLbl);
                jmp(ptr[rip + contAddrLbl]);
                L(zeroLbl);
                jmp(ptr[rip + zeroAddrLbl]);

                L(contAddrLbl);
                dq(contNonzeroAddr);
                L(zeroAddrLbl);
                dq(contZeroAddr);
            }
        };

        Code code(unrestricted_cast<std::uintptr_t>(jmpdstNonZero), unrestricted_cast<std::uintptr_t>(jmpdstZero));
        code.ready();

        _MESSAGE("installing fix");
        auto trampoline = SKSE::GetTrampoline();
        if (!trampoline->Write5Branch(unrestricted_cast<std::uintptr_t>(crashaddr), reinterpret_cast<std::uintptr_t>(code.getCode())))
            return false;

        _MESSAGE("success");
        return true;
    }

   class TreeReflectionsPatch
    {
    public:
        static bool Install()
        {
            const auto handle = GetModuleHandleA("d3dcompiler_46e.dll");

            if (handle)
            {
                _WARNING("enb detected - disabling fix, please use ENB's tree reflection fix instead");
                return true;
            }

            _MESSAGE("patching BSDistantTreeShader vfunc 3");
            struct Patch :  SKSE::CodeGenerator
            {
                Patch(std::uintptr_t a_target) : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;

                    // current: if(bUseEarlyZ) v3 |= 0x10000u;
                    // goal: if(bUseEarlyZ || v3 == 0) v3 |= 0x10000u;
                    // if (bUseEarlyZ)
                    // .text:0000000141318C50                 cmp     cs:bUseEarlyZ, r13b
                    // need 6 bytes to branch jmp so enter here
                    // enter 1318C57
                    // .text:0000000141318C57                 jz      short loc_141318C5D
                    jnz("CONDITION_MET");
                    // edi = v3
                    // if (v3 == 0)
                    test(edi, edi);
                    jnz("JMP_OUT");
                    // .text:0000000141318C59                 bts     edi, 10h
                    L("CONDITION_MET");
                    bts(edi, 0x10);
                    L("JMP_OUT");
                    // exit 1318C5D
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_target + 0x6);
                }
            };
            REL::Offset<std::uintptr_t> target (0x1343d40 + 0x37 );
            Patch patch(target.GetAddress());
            patch.ready();

            auto trampoline = SKSE::GetTrampoline();
            if (!trampoline->Write6Branch(unrestricted_cast<std::uintptr_t>(target.GetAddress()), reinterpret_cast<std::uintptr_t>(patch.getCode()))) {
                return false;
            }
            _MESSAGE("success");

            return true;
        }
    };

    bool PatchTreeReflections()
    {
        _MESSAGE("- blocky tree reflections fix -");

        return TreeReflectionsPatch::Install();
    }

    bool PatchFaceGenMorphDataHeadNullptrCrash()
    {
        // written for VR

        _VMESSAGE("- fix for crash in FaceGenMorphDataHead -");
        // loc_1403E6250
        /*
        .....
        .text:00000001403E627D        xor     r15d, r15d
        .text:00000001403E6280        test    rdx, rdx           // rdx == 0 causes the first crash in a few instructions
        .text:00000001403E6283    +-- jz      short loc_1403E6299
        .text:00000001403E6285    |   lea     rdx, unk_142FCCBF8 // <-- rdx != 0, this is the good case and
        .text:00000001403E628C    |   mov     rcx, rax           //      we want to continue here if everything is well
        .text:00000001403E628F    |   call    NiAVObject__GetExtraDataByName_140CA6080
        .text:00000001403E6294    |   mov     rbx, rax         // <--- This sometimes has rax == 0 -> second possibility how rbx ends up as 0
        .text:00000001403E6297  +-|-- jmp     short loc_1403E629C
        .text:00000001403E6299  | +-->mov     rbx, r15         // <--- if this is executed it'll crash on the next line since r15 is 0 here
        .text:00000001403E629C  +---> mov     eax, [rbx+24h]
        .text:00000001403E629F        comiss  xmm6, cs:dword_14152259C
        .text:00000001403E62A6        jb      loc_1403E668F  // ---> This jumps to the function stack restore + return point
        */
        // What can we do here? There are two ways for rbx to end up NULL,
        // so we need to move the last mov followed by comiss into the trampoline
        // and wrap that into a check for rbx != 0.
        // The 'mov rbx, r15' is also weird. Only the lower 32 bits of r15 are guaranteed to be 0,
        // but then rbx is used as an address. So it's probably better to replace that instruction
        // with 'xor ebx, ebx' to make sure our inserted check will catch it in all cases.

        // TODO: There is probably a cleaner way to patch this without moving a float constant into the trampoline, but this is tested and works ok

        REL::Offset<std::uintptr_t> baseFuncAddr = 0x3E6250;

        // check that test rdx,rdx and jz short are there
        uint8_t* testrdx = (uint8_t*)(uintptr_t)(baseFuncAddr.GetAddress() + 0x30);
        static const uint8_t expectedTestRdxJz[] = { 0x48, 0x85, 0xD2, 0x74 };
        if (std::memcmp(testrdx, expectedTestRdxJz, sizeof(expectedTestRdxJz)))
            return false;

        // Where does the jz short go?
        const int8_t disp8 = testrdx[sizeof(expectedTestRdxJz)]; // jz short displacement (should be 0x14)
        uint8_t* jmpdstZero = testrdx + sizeof(expectedTestRdxJz) + disp8 + 1;

        // Is the zero case jump target 'mov rbx, r15'? (This is patched later)
        static const uint8_t expectedMovRbx[] = { 0x49, 0x8B, 0xDF };
        if (std::memcmp(jmpdstZero, expectedMovRbx, sizeof(expectedMovRbx)))
            return false;

        // Find the where to insert the trampoline jump and where the jump to the exit is
        uint8_t* pUnsafeMov = jmpdstZero + sizeof(expectedMovRbx); // this is the mov that potentially causes a crash
        if (pUnsafeMov[0] != 0x8B)
            return false;

        static const uint8_t copysize = 3; // mov -- can't just copy the comiss because it uses a relative address :<
        const uint8_t* jbExit = pUnsafeMov + copysize + 7; // + comiss

        // Make sure jb => exit is there as expected
        static const uint8_t expectedJB[] = { 0x0f, 0x82 };
        if (std::memcmp(jbExit, expectedJB, sizeof(expectedJB)))
            return false;

        // Get the jump offset
        int32_t jbOffs = *(int32_t*)(jbExit + sizeof(expectedJB));

        // Figure out where the function exit location is
        const uint8_t* nextInst = jbExit + 2 + sizeof(int32_t);
        const uint8_t* exitLoc = nextInst + jbOffs;

        struct Code : SKSE::CodeGenerator
        {
            Code(const uint8_t* originalcode, std::uintptr_t pContLoc, std::uintptr_t pExitLoc) : SKSE::CodeGenerator()
            {
                Xbyak::Label zeroLbl, fltConstL;

                test(rbx, rbx);
                jz(zeroLbl);

                db(originalcode, copysize); // the unsafe mov
                comiss(xmm6, ptr[rip + fltConstL]);

                jmp(ptr[rip]);
                dq(pContLoc);

                L(zeroLbl);
                jmp(ptr[rip]);
                dq(pExitLoc);

                L(fltConstL);
                dd(0x0BF800000); // -1.0f
            }
        };

        Code code(pUnsafeMov, unrestricted_cast<std::uintptr_t>(jbExit), unrestricted_cast<std::uintptr_t>(exitLoc));
        code.finalize();

        _MESSAGE("installing fix");
        auto trampoline = SKSE::GetTrampoline();
        if (!trampoline->Write5Branch(unrestricted_cast<std::uintptr_t>(pUnsafeMov), reinterpret_cast<std::uintptr_t>(code.getCode()))) {
            return false;
        }
            
        // nop out the rest of the original code so that the diasm/debugger is less confused
        static const uint8_t nop5[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        SKSE::SafeWriteBuf(uintptr_t(pUnsafeMov) + 5, nop5, sizeof(nop5));

        // Fix rbx clear (3 bytes to cover)
        struct Patch : SKSE::CodeGenerator
        {
            Patch()
            {
                xor_(ebx, ebx); // 2 bytes
                nop();          // 1 byte
            }
        };
        Patch patch;
        patch.finalize();
        assert(patch.getSize() == 3);
        SKSE::SafeWriteBuf((uintptr_t)jmpdstZero, patch.getCode(), 3);

        _VMESSAGE("success");
        return true;
    }
}


