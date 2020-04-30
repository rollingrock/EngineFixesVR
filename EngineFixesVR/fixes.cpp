#include "fixes.h"

namespace fixes
{
    bool PatchAll()
    {
            PatchMemoryAccessErrors();

            PatchLipSync();

            PatchGHeapLeakDetectionCrash();
        
        //if (config::fixCellInit)
        if (false)  // Need to verify that SSE commonlib objects still are ok to use otherwise this seems to work
            PatchCellInit();

            PatchAnimationLoadSignedCrash();

            PatchBSLightingAmbientSpecular();

            PatchCalendarSkipping();

            PatchConjurationEnchantAbsorbs();

            PatchEquipShoutEventSpam();

     //       PatchPerkFragmentIsRunning();

            PatchRemovedSpellBook();

        return true;
    }
}
