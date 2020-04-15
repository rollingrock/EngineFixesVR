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










}

