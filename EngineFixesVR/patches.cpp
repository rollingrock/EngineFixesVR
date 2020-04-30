#include "patches.h"

namespace patches
{
    bool PatchAll()
    {
 /*       if (config::patchDisableChargenPrecache)
            PatchDisableChargenPrecache();
            */
            PatchFormCaching();

            PatchEnableAchievementsWithMods();

            PatchMaxStdio();

        //if (config::patchRegularQuicksaves)
        //    PatchRegularQuicksaves();

        //if (config::patchSaveAddedSoundCategories)
        //    PatchSaveAddedSoundCategories();

        //if (config::patchScrollingDoesntSwitchPOV)
        //    PatchScrollingDoesntSwitchPOV();

        //if (config::patchSleepWaitTime)
        //    PatchSleepWaitTime();

            PatchTreeLODReferenceCaching();

        //if (config::patchWaterflowAnimation)
        //    PatchWaterflowAnimation();

        //if (config::experimentalSaveGameMaxSize)
        //    PatchSaveGameMaxSize();

        //if (config::experimentalTreatAllModsAsMasters)
        //    PatchTreatAllModsAsMasters();

        return true;
    }
}
