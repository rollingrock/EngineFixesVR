#pragma once

#include "config.h"
#include "offsets.h"

namespace fixes
{
    bool PatchMemoryAccessErrors();
    bool PatchLipSync();
    bool PatchGHeapLeakDetectionCrash();
    bool PatchCellInit();
    bool PatchAnimationLoadSignedCrash();
    bool PatchBSLightingAmbientSpecular();
    bool PatchDoublePerkApply();
    bool PatchCalendarSkipping();
    bool PatchConjurationEnchantAbsorbs();
    bool PatchEquipShoutEventSpam();
    bool PatchPerkFragmentIsRunning();
    bool PatchRemovedSpellBook();
    bool PatchFixAbilityConditionBug();
    bool PatchFixBuySellStackSpeechGain();
    bool PatchShadowSceneCrash();

    bool PatchAll();
}
