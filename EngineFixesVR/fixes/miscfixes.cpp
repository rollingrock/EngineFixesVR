#include <array>
#include <utility>

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Trampoline.h"

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

            REL::Offset<std::uintptr_t> vtbl = REL::Module::BaseAddr() + 0x18fea78;    // SE is 12d38f3   VR is 18fea78
            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 0x1338400;   // SE is 12f2020  VR is 1338400
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

            REL::Offset<std::uintptr_t> vtbl = REL::Module::BaseAddr() + 0x15ebf50;
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

            REL::Offset<std::uintptr_t> target = REL::Module::BaseAddr() + 0x13574f0;  // SE is 13251c0   VR is 13574f0

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
            
            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 0x201d80;   // SE is 1f12a0  VR is 201d80
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
            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 0x105cbb0;   //SE is fffeb0  VR is 105cbb0

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
            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 273830;  // SE is 262290  VR is 273830
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
            REL::Offset<std::uintptr_t> target =  REL::Module::BaseAddr() + 0xba17a0 + 0x91;   // SE is b66930   VR is ba17a0
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

            REL::Offset<std::uintptr_t> addAmbientSpecularToSetupGeometry = REL::Module::BaseAddr() + 0x1339bb9;   // SE is 12f2bb0  VR is 1338f60   0xBAD in SE goes to 1339bb9 which is offset 0xC59
            REL::Offset<std::uintptr_t> ambientSpecularAndFresnel = REL::Module::BaseAddr() + 0x342317c;           // SE is 1e0dfcc  VR is 342317c
            REL::Offset<std::uintptr_t> disableSetupMaterialAmbientSpecular = REL::Module::BaseAddr() + 0x1338400 + 0x713;               // SE is 12f2020  VR is 1338400

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

            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 0x5ad8f0;  // SE is 5a6230  VR is 5ad8f0

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
            REL::Offset<std::uintptr_t> vtbl = REL::Module::BaseAddr() + 0x1598b30;     // SE is 15217e0   VR is 1598b30
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

            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 0x63b290;  // SE is 6323c0 VR is 63b290

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
            REL::Offset<std::uintptr_t> funcBase = REL::Module::BaseAddr() + 0x2ecb20;  // SE is 2db610 VR is 2ecb20
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
            REL::Offset<std::uintptr_t> vtbl = REL::Module::BaseAddr() + 0x15c9e68;   // SE is 15592b8  VR is 15c9e68
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
}

