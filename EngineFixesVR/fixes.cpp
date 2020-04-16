#include "fixes.h"

namespace fixes
{
    bool PatchAll()
    {
        if (config::fixMemoryAccessErrors)
            PatchMemoryAccessErrors();

        if (config::fixLipSync)
            PatchLipSync();

        if (config::fixGHeapLeakDetectionCrash)
            PatchGHeapLeakDetectionCrash();
        
        //if (config::fixCellInit)
        if (false)  // Need to verify that SSE commonlib objects still are ok to use otherwise this seems to work
            PatchCellInit();

        if (config::fixAnimationLoadSignedCrash)
            PatchAnimationLoadSignedCrash();

        if (config::fixBSLightingAmbientSpecular)    // !!!!Not crashing after fixing assembly registers.   However please please please go verify stack pointer offsets are ok before releasing!!!!
            PatchBSLightingAmbientSpecular();

        if (config::fixCalendarSkipping)
            PatchCalendarSkipping();

        if (config::fixConjurationEnchantAbsorbs)
            PatchConjurationEnchantAbsorbs();

        if (config::fixEquipShoutEventSpam)
            PatchEquipShoutEventSpam();

        if (config::fixPerkFragmentIsRunning)
            PatchPerkFragmentIsRunning();

        if (config::fixRemovedSpellBook)
            PatchRemovedSpellBook();

        return true;
    }
}
