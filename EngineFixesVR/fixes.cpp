#include "fixes.h"

namespace fixes
{
    bool PatchAll()
    {
        if (config::fixMemoryAccessErrors) {
            PatchMemoryAccessErrors();
        }

        if (config::fixLipSync) {
            PatchLipSync();
        }

        if (config::fixGHeapLeakDetectionCrash) {
            PatchGHeapLeakDetectionCrash();
        }
        
        if (config::fixCellInit) {
            PatchCellInit();
        }

        if (config::fixAnimationLoadSignedCrash) {
            PatchAnimationLoadSignedCrash();
        }

        if (config::fixBSLightingAmbientSpecular) {
            PatchBSLightingAmbientSpecular();
        }

        if (config::fixCalendarSkipping) {
            PatchCalendarSkipping();
        }

        if (config::fixConjurationEnchantAbsorbs) {
            PatchConjurationEnchantAbsorbs();
        }

        if (config::fixEquipShoutEventSpam) {
            PatchEquipShoutEventSpam();
        }

        if (config::fixPerkFragmentIsRunning) {
            PatchPerkFragmentIsRunning();
        }

        if (config::fixRemovedSpellBook) {
            PatchRemovedSpellBook();
        }

        return true;
    }
}
