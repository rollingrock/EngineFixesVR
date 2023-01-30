#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/Trampoline.h"

#include "skse64/GameData.h"

#include "include/toml.hpp"

#include "patches.h"
#include "utils.h"

namespace patches
{
//    REL::Relocation<bool(void*)> orig_INIPrefSettingCollection_Unlock;
    typedef bool (*orig_INIPrefSettingCollection_Unlock_)(void*);
    REL::Offset<orig_INIPrefSettingCollection_Unlock_> orig_INIPrefSettingCollection_Unlock(0xd80bc0);

    typedef void (*SetCategoryVolume_)(RE::BSISoundCategory* sound, float vol);
    REL::Offset<SetCategoryVolume_> SetCategoryVolume(0x2df5a0);

    constexpr std::string_view FILE_NAME = "Data/SKSE/Plugins/EngineFixesVR_SNCT.toml";

    toml::table& get_store()
    {
        static toml::table store;
        return store;
    }

    bool hk_INIPrefSettingCollection_Unlock(void* thisPtr)
    {
        _MESSAGE("in setting func");
        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler)
        {
            auto& store = get_store();

            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                if (soundCategory->IsMenuCategory())
                {
                    auto fullName = soundCategory->GetFullName();
                    fullName = fullName ? fullName : "";
                  //  logger::trace(FMT_STRING("processing {}"), fullName);
                 //   logger::trace("menu flag set, saving");
                    auto localFormID = soundCategory->formID & 0x00FFFFFF;
                    // esl
                    if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                    {
                        localFormID = localFormID & 0x00000FFF;
                    }
                    auto srcFile = soundCategory->GetDescriptionOwnerFile();
                 //   logger::trace(FMT_STRING("plugin: {} form id: {:08X}"), srcFile->fileName, localFormID);

                    char localFormIDHex[] = "DEADBEEF";
                    sprintf_s(localFormIDHex, std::extent_v<decltype(localFormIDHex)>, "%08X", localFormID);

                    auto table = store.get_as<toml::table>(srcFile->fileName);
                    if (!table)
                    {
                        table =
                            store.insert_or_assign(
                                srcFile->fileName,
                                toml::table{})
                            .first->second.as_table();
                    }

                    table->insert_or_assign(localFormIDHex, static_cast<double>(soundCategory->volumeMult));
                }
            }

            std::ofstream file{ FILE_NAME.data(), std::ios_base::out | std::ios_base::trunc};
            if (!file)
            {
         //       logger::trace("warning: unable to save snct ini");
            }

            file << store;

        //    logger::trace("SNCT save: saved sound categories");
        }
        else
        {
        //    _MESSAGE("SoundSave: datahandler not ready so not saving with this call");
        }

        return orig_INIPrefSettingCollection_Unlock(thisPtr);
    }

    void LoadVolumes()
    {
        _MESSAGE("game has loaded, setting volumes");

        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler)
        {
            auto& store = get_store();

            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                auto localFormID = soundCategory->formID & 0x00FFFFFF;
                // esl
                if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                {
                    localFormID = localFormID & 0x00000FFF;
                }
                char localFormIDHex[] = "DEADBEEF";
                sprintf_s(localFormIDHex, std::extent_v<decltype(localFormIDHex)>, "%08X", localFormID);

                auto srcFile = soundCategory->GetDescriptionOwnerFile();
                const auto vol = store[srcFile->fileName][localFormIDHex].as_floating_point();

                if (vol)
                {
            //        logger::trace(FMT_STRING("setting volume for formid {:08X}"), soundCategory->formID);
             //       const REL::Relocation<bool (**)(RE::BSISoundCategory*, float)> SetVolume{ REL::ID(236602), 0x8 * 0x3 };
               //     SetCategoryVolume(soundCategory, static_cast<float>(vol->get()));

                  //  _MESSAGE("setting volume for mod");
                    SetCategoryVolume(soundCategory, static_cast<float>(vol->get()));

                }
            }
        }
    }

    bool PatchSaveAddedSoundCategories()
    {
        _MESSAGE("- save added sound categories -");

        try
        {
            auto& store = get_store();
            store = toml::parse_file(FILE_NAME);
        }
        catch (const std::exception&)
        {
        }

   //     logger::trace("hooking vtbls");
//        REL::Relocation<std::uintptr_t> vtbl{ REL::ID(230546) };  // INIPrefSettingCollection
//        orig_INIPrefSettingCollection_Unlock = vtbl.write_vfunc(0x6, hk_INIPrefSettingCollection_Unlock);
 //       logger::trace("success");

        REL::Offset<std::uintptr_t> vfunc(0x15ace58);
        SKSE::SafeWrite64(vfunc.GetAddress(), (UInt64)&hk_INIPrefSettingCollection_Unlock);
        _MESSAGE("- success -");
        return true;
    }
}
