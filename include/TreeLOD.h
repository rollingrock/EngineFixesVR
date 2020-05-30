#pragma once
#include "RE/BSTArray.h"


class DistantTreeLOD {
	public:
	struct instance
	{
		uint32_t formid; //0x0000
		char pad_0004[10]; //0x0004
		uint16_t alpha; //0x000E
		bool hidden; //0x0010
		int16_t padding; //0x0011
	//	int8_t N00000173; //0x0013
	}; //Size: 0x0014

	struct treeGroup
	{
		char pad_0000[8]; //0x0000
		instance* instance_ptr; //0x0008
		char pad_0010[8]; //0x0010
		uint16_t numInsts; //0x0018
		char pad_001A[8]; //0x001A
		uint16_t N000001A1; //0x0022
		bool updateShader; //0x0024
	}; //Size: 0x0025

	struct treeGroupArray
	{
		treeGroup* groups;
	};
	struct TreeLOD
	{
		treeGroupArray* grp_ptr; //0x0000
		uint64_t pad1; //0x0008
		uint16_t SizeofTreeGroup; //0x0010
		char pad_000A[110]; //0x0012
		uint16_t padd_000B; //0x0080
		bool AllVisible; //0x0082
	}; 
};
