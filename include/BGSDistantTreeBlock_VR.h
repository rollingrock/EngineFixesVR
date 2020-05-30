#pragma once

#include "RE/BSTArray.h"

namespace RE
{
    class BGSDistantTreeBlock_VR/* : public BSResourceEntry */
    {
    public:
        struct LODGroupInstance
        {
            UInt32 id;	// Only the lower 24 bits used
            char _pad[0xA];
            UInt16 alpha;		// This is Float2Word(fAlpha)
            bool hidden;		// Alpha <= 0.0f or set by object flags
        };

        struct LODGroup
        {
            char _pad[8];
            BSTArray<LODGroupInstance> m_LODInstances;
            char _pad2[4];
            bool shaderPropertyUpToDate;
        };

        struct ResourceData
        {
            BSTArray<LODGroup*> m_LODGroups;
            char _pad[106];
            bool allVisible;
        };

        // struct ResourceData @ 0x28
    };
}