#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/Trampoline.h"
#include "RE/NiNode.h"

#include "tbb/concurrent_hash_map.h"

#include "skse64/GameData.h"
#include "skse64/NiObjects.h"

#include "BGSDistantTreeBlock_VR.h"
#include "TreeLOD.h"
#include "patches.h"

namespace patches
{
    std::uintptr_t baseAddr = REL::Module::BaseAddr();

    typedef void (*UpdateBlockVisibility_)(RE::BGSDistantTreeBlock* data);
    REL::Offset<UpdateBlockVisibility_> UpdateBlockVisibility_orig(0x4b80a0);    // 4a8090 in SSE

    typedef uint16_t (*Float2Half_)(float f);
    REL::Offset<Float2Half_> Float2Half(0x26f3b0);    // was d426f0

    typedef RE::TESForm* (*_LookupFormByID)(uint32_t id);
    REL::Offset<_LookupFormByID> LookupFormByID(0x1a3f60);    // was 194230

    tbb::concurrent_hash_map<uint32_t, RE::TESObjectREFR*> referencesFormCache;

    void InvalidateCachedForm(uint32_t FormId)
    {
        referencesFormCache.erase(FormId & 0x00FFFFFF);
    }

    void hk_UpdateBlockVisibility(DistantTreeLOD::TreeLOD* data)
    {
        DistantTreeLOD::treeGroupArray* grp_array = data->grp_ptr;

        for (auto i = 0; i < data->SizeofTreeGroup; ++i) {
            DistantTreeLOD::treeGroup* group = (grp_array + i)->groups;

            if ((group == 0x0) || (group->numInsts == 0)) {
                break;
            }

            DistantTreeLOD::instance* instance = group->instance_ptr;
            for (auto j = 0; j < group->numInsts; ++j) {
                const uint32_t maskedFormId = (instance+j)->formid & 0x00FFFFFF;

                RE::TESObjectREFR* refrObject = nullptr;

                decltype(referencesFormCache)::accessor accessor;

                if (referencesFormCache.find(accessor, maskedFormId))
                {
                    refrObject = accessor->second;
                }
                else
                {
                // Find first valid tree object by ESP/ESM load order
                // This shit is broken from how SSE datahandler is and the VR.   Guessing VR is using a pre-CC version.  Either way re-implemetning with SKSE GameData.h
                        RE::TESDataHandler *dataHandler = RE::TESDataHandler::GetSingleton();

                        DataHandler* dh = DataHandler::GetSingleton();

                        for (uint32_t i = 0; i < dh->modList.loadedModCount; i++)
                        {
                            RE::TESForm* form = LookupFormByID((i << 24) | maskedFormId);
                            if (form) {
                                refrObject = form->AsReference();
                            }

                            if (refrObject)
                            {
                                auto baseObj = refrObject->GetBaseObject();
                                if (baseObj)
                                {
                                    using STATFlags = RE::TESObjectSTAT::RecordFlags;
                                    // Checks if the form type is TREE (TESObjectTREE) or if it has the kHasTreeLOD flag (TESObjectSTAT)
                                    if (baseObj->formFlags & STATFlags::kHasTreeLOD || baseObj->Is(RE::FormType::Tree)) {
                            //            _MESSAGE("made it here");
                                        break;
                                    }
                                }
                            }

                            refrObject = nullptr;
                        }

                        // Insert even if it's a null pointer
                        referencesFormCache.insert(std::make_pair(maskedFormId, refrObject));
                }
                bool fullyHidden = false;
                float alpha = 1.0f;

                if (refrObject)
                {
                    auto obj3D = refrObject->Get3D();
                    auto cell = refrObject->GetParentCell();

                    if (obj3D && !obj3D->GetAppCulled() && cell->IsAttached())
                    {
                        static auto bEnableStippleFade = RE::GetINISetting("bEnableStippleFade:Display");  // ini settings are kept in a linked list, so we'll cache it
                        if (bEnableStippleFade->GetBool())
                        {
                            const auto fadeNode = obj3D->AsFadeNode();
                            if (fadeNode)
                            {
                                alpha = 1.0f - fadeNode->currentFade;
                                if (alpha <= 0.0f)
                                    fullyHidden = true;
                            }
                        }
                        else
                        {
                            // No alpha fade - LOD trees will instantly appear or disappear
                            fullyHidden = true;
                        }
                    }

                    if (refrObject->IsInitiallyDisabled() || refrObject->IsDeleted())
                        fullyHidden = true;
                }

                const uint16_t halfFloat = Float2Half(alpha);

                if ((instance+j)->alpha != halfFloat)
                {
                    (instance+j)->alpha = halfFloat;
                    group->updateShader = false;
                }

                if ((instance+j)->hidden != fullyHidden)
                {
                    (instance+j)->hidden = fullyHidden;
                    group->updateShader = false;
                }

                if (fullyHidden)
                    data->AllVisible = false;
            }
        }

    }


    void testitout(DistantTreeLOD::TreeLOD* data) {

        _VMESSAGE("num of treegroup %016I64X = %d", data->grp_ptr, data->SizeofTreeGroup);

        DistantTreeLOD::treeGroupArray* grp_array = data->grp_ptr;
        
        for (auto i = 0; i < data->SizeofTreeGroup; ++i) {
            DistantTreeLOD::treeGroup* grp = (grp_array + i)->groups;

            _VMESSAGE("grp is %016I64X", grp);

            if (grp == 0x0) {
                break;
            }
            
            DistantTreeLOD::instance* inst = grp->instance_ptr;
            for (auto j = 0; j < grp->numInsts; ++j) {
                _VMESSAGE("instance is %016I64X : %x : %x", &inst[j], (inst+j)->formid, (inst+j)->alpha);
                (inst + j)->alpha = 0x0;
                (inst + j)->hidden = 1;
            }
            grp->updateShader = 0;
        }

        data->AllVisible = 0;

        return;
    }

    /*void ghidra_UpdateBlockVisibility(uintptr_t* data) {
        float alpha_init = 0.0f;
        float fVar8 = 0.0f;
        float alpha = 0.0f;

        uintptr_t treegrp_ptr;
        uintptr_t instance;
        uintptr_t* plVar5;
        uintptr_t lVar2;
        uint32_t local_res10[2];
        uint32_t* puVar1;
        uintptr_t* plVar3;
        uint32_t* puVar4;

        char cVar6, cVar7;

        bool hidden;

        int mod_number = 0;
        int nummods = 0;

        uint32_t masked_id = 0;

        uintptr_t baseAddr = REL::Module::BaseAddr();
        uintptr_t local_res18 = baseAddr + 0x1f88fb0;
        uintptr_t func_140c42150 = baseAddr + 0xc42150;
        uintptr_t func_140c41370 = baseAddr + 0xc41370;

        typedef void func1(uintptr_t*);
        typedef void func2(uint32_t*, uintptr_t);
        func1* f1 = (func1*)func_140c42150;
        f1((uintptr_t*)local_res18);

        uint64_t cur_instance = 0;
        int i = 0;

        if (*(int*)(data + 2) != 0) {
            alpha_init = 1.0f;

            do {
                treegrp_ptr = *(uintptr_t*)(*data + cur_instance * 8);
                cur_instance = 0;
                alpha = alpha_init;

                if (*(int*)(treegrp_ptr + 0x18) != 0) {
                    do {
                        instance = *(uintptr_t*)(treegrp_ptr + 8);
                        plVar5 = (uintptr_t*)0x0;
                        mod_number = 0;
                        DataHandler* dh = DataHandler::GetSingleton();
                        nummods = dh->modList.loadedModCount;

                        if (nummods != 0) {
                            do {
                                uintptr_t p = baseAddr + 0x1f88b18;
                                lVar2 = *reinterpret_cast<uintptr_t*>(p);

                                if (plVar5 != (uintptr_t*)0x0) {
                                    break;
                                }

                                masked_id = *(uint32_t*)(instance + cur_instance * 0x14) & 0xffffff | mod_number << 0x18;
                                if (*reinterpret_cast<uint32_t*>(p) != 0) {
                                    func2* f2 = (func2*)func_140c41370;
                                    f2(local_res10, (uintptr_t)masked_id);

                                    if (*(uintptr_t*)(lVar2 + 0x28) != 0) {
                                        puVar4 = (uint32_t*)(*(uintptr_t*)(lVar2 + 0x28) + (uintptr_t)(*(int*)(lVar2 + 0xc) - 1 & local_res10[0]) * 0x18);
                                        puVar1 = *(uint32_t**)(puVar4 + 4);

                                        while (puVar1 != (uint32_t*)0x0) {
                                            if (*puVar4 == masked_id) {
                                                plVar5 = (uintptr_t*)(**(uint32_t**)(**(uintptr_t**)(puVar4 + 2) + 0x160));
                                                lVar2 = plVar5[8];
                                                if ((*(uintptr_t**)(puVar4 + 2) != (uintptr_t*)0x0) && (plVar5 == (uintptr_t*)0x0 || lVar2 != 0 && (((*(uint32_t*)(lVar2 + 0x10) >> 6 & 1) != 0 || (*(char*)(lVar2 + 0x1a) == '&'))))) {
                                                    break;
                                                }
                                            }

                                        puVar4 = *(uint32_t**)(puVar4 + 4);
                                        puVar1 = (uint32_t*)((uintptr_t)puVar4 - *(uintptr_t*)(lVar2 + 0x18));
                                        }
                                    }
                                }
                                plVar5 = (uintptr_t*)0x0;
                                mod_number++;
                                masked_id = dh->modList.loadedModCount;
                            } while (mod_number < masked_id);
                        }
                        cVar6 = '\0';
                        hidden = *(char*)(instance + 0x10 + cur_instance * 0x14);
                        alpha_init = alpha;
                        cVar7 = '\0';
                        
                        if (plVar5 != (uintptr_t*)0x0) {
                            lVar2 = **(uintptr_t**)(*plVar5 + 0x380);
                            if ((lVar2 != 0) && ((*(uintptr_t*)(lVar2 + 0x10c) & 1) == 0)) {
                                char loc = (char)(*(uintptr_t*)(baseAddr + 0x1ed3e10));
                                if (loc == '\0') {
                                    if (*(char*)(plVar5[0xc] + 0x44) == '\a') {
                                        cVar6 = '\x01';
                                    }
                                }
                                else {
                                    if (*(char*)(plVar5[0xc] + 0x44) == '\a') {
                                        plVar3 = (uintptr_t*)(**(uint32_t**)(*plVar5 + 0x380))(plVar5);
                                    }
                                }

                            }
                        }
                    }
                } while (mod_number < *(int*)(treegrp_ptr + 0x18));
            } while (i < *(int*)(data + 2));
        }

        


    }*/


    bool PatchTreeLODReferenceCaching()
    {
        _VMESSAGE("- Tree LOD Reference Caching -");

        _VMESSAGE("detouring UpdateLODAlphaFade");
        _VMESSAGE("gimmie addr %016I64X", unrestricted_cast<std::uintptr_t>(&hk_UpdateBlockVisibility));        
        _VMESSAGE("gimmie addr %016I64X", UpdateBlockVisibility_orig.GetAddress());


        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write6Branch(UpdateBlockVisibility_orig.GetAddress(), unrestricted_cast<std::uintptr_t>(&hk_UpdateBlockVisibility));
        _VMESSAGE("success");

        return true;
    }
}
