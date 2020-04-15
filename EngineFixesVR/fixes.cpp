#include "fixes.h"

namespace fixes
{
    bool PatchAll()
    {
        if (config::fixMemoryAccessErrors)
            PatchMemoryAccessErrors();

        return true;
    }
}
