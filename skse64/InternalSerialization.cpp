#include <map>
#include <algorithm>
#include <string>
#include "common/IFileStream.h"
#include "skse64/PluginAPI.h"
#include "skse64/InternalSerialization.h"
#include "skse64/GameData.h"
#include "skse64/Serialization.h"
#include "PapyrusEvents.h"
#include "PapyrusDelayFunctors.h"
#include "PapyrusObjects.h"

// Internal

static UInt8	s_savefileIndexMap[0xFF];
static UInt8	s_numSavefileMods = 0;

void LoadModList(SKSESerializationInterface * intfc)
{
	_MESSAGE("Loading mod list:");

	DataHandler * dhand = DataHandler::GetSingleton();

	char name[0x104] = { 0 };
	UInt16 nameLen = 0;

	intfc->ReadRecordData(&s_numSavefileMods, sizeof(s_numSavefileMods));
	for (UInt32 i = 0; i < s_numSavefileMods; i++)
	{
		intfc->ReadRecordData(&nameLen, sizeof(nameLen));
		intfc->ReadRecordData(&name, nameLen);
		name[nameLen] = 0;

		UInt8 newIndex = dhand->GetLoadedModIndex(name);
		s_savefileIndexMap[i] = newIndex;
		_MESSAGE("\t(%d -> %d)\t%s", i, newIndex, &name);
	}
}

void SaveModList(SKSESerializationInterface * intfc)
{
	DataHandler * dhand = DataHandler::GetSingleton();
	UInt8 modCount = dhand->modList.loadedModCount;

	intfc->OpenRecord('MODS', 0);
	intfc->WriteRecordData(&modCount, sizeof(modCount));

	_MESSAGE("Saving mod list:");

	for (UInt32 i = 0; i < modCount; i++)
	{
		ModInfo * modInfo = dhand->modList.loadedMods[i];
		UInt16 nameLen = strlen(modInfo->name);
		intfc->WriteRecordData(&nameLen, sizeof(nameLen));
		intfc->WriteRecordData(modInfo->name, nameLen);
		_MESSAGE("\t(%d)\t%s", i, &modInfo->name);
	}
}

UInt8 ResolveModIndex(UInt8 modIndex)
{
	return (modIndex < s_numSavefileMods) ? s_savefileIndexMap[modIndex] : 0xFF;
}

static UInt8	s_saveLightfileIndexMap[0xFFF];
static UInt8	s_numSaveLightfileMods = 0;

void LoadLightModList(SKSESerializationInterface * intfc)
{
	_MESSAGE("Loading light mod list:");

	DataHandler * dhand = DataHandler::GetSingleton();

	char name[0x104] = { 0 };
	UInt16 nameLen = 0;

	intfc->ReadRecordData(&s_numSaveLightfileMods, sizeof(s_numSaveLightfileMods));
	for (UInt32 i = 0; i < s_numSaveLightfileMods; i++)
	{
		intfc->ReadRecordData(&nameLen, sizeof(nameLen));
		intfc->ReadRecordData(&name, nameLen);
		name[nameLen] = 0;

		UInt8 newIndex = dhand->GetLoadedLightModIndex(name);
		s_saveLightfileIndexMap[i] = newIndex;
		_MESSAGE("\t(%d -> %d)\t%s", i, newIndex, &name);
	}
}

void SaveLightModList(SKSESerializationInterface * intfc)
{
	DataHandler * dhand = DataHandler::GetSingleton();
	UInt8 modCount = 0;

	intfc->OpenRecord('LMOD', 0);
	intfc->WriteRecordData(&modCount, sizeof(modCount));
}

UInt16 ResolveLightModIndex(UInt16 modIndex)
{
	return (modIndex < s_numSaveLightfileMods) ? s_saveLightfileIndexMap[modIndex] : 0xFFFF;
}

//// Callbacks

void Core_RevertCallback(SKSESerializationInterface * intfc)
{
	g_menuOpenCloseRegs.Clear();
	g_inputKeyEventRegs.Clear();
	g_inputControlEventRegs.Clear();
	g_modCallbackRegs.Clear();
	g_crosshairRefEventRegs.Clear();
	g_cameraEventRegs.Clear();
	g_actionEventRegs.Clear();
	g_ninodeUpdateEventRegs.Clear();

	SKSEDelayFunctorManagerInstance().OnRevert();

	SKSEObjectStorageInstance().ClearAndRelease();
}

void Core_SaveCallback(SKSESerializationInterface * intfc)
{
	using Serialization::SaveClassHelper;

	SaveModList(intfc);
	SaveLightModList(intfc);

	_MESSAGE("Saving menu open/close event registrations...");
	g_menuOpenCloseRegs.Save(intfc, 'MENR', 1);

	_MESSAGE("Saving key input event registrations...");
	g_inputKeyEventRegs.Save(intfc, 'KEYR', 1);

	_MESSAGE("Saving control input event registrations...");
	g_inputControlEventRegs.Save(intfc, 'CTLR', 1);

	_MESSAGE("Saving mod callback event registrations...");
	g_modCallbackRegs.Save(intfc, 'MCBR', 1);

	_MESSAGE("Saving crosshair ref event registrations...");
	g_crosshairRefEventRegs.Save(intfc, 'CHRR', 1);

	_MESSAGE("Saving camera event registrations...");
	g_cameraEventRegs.Save(intfc, 'CAMR', 1);

	_MESSAGE("Saving actor action event registrations...");
	g_actionEventRegs.Save(intfc, 'AACT', 1);

	_MESSAGE("Saving NiNode update event registrations...");
	g_ninodeUpdateEventRegs.Save(intfc, 'NINU', 1);

	_MESSAGE("Saving SKSEPersistentObjectStorage data...");
	SaveClassHelper(intfc, 'OBMG', SKSEObjectStorageInstance());

	_MESSAGE("Saving SKSEDelayFunctorManager data...");
	SaveClassHelper(intfc, 'DFMG', SKSEDelayFunctorManagerInstance());
}

void Core_LoadCallback(SKSESerializationInterface * intfc)
{
	UInt32 type, version, length;

	while (intfc->GetNextRecordInfo(&type, &version, &length))
	{
		switch (type)
		{
		// Mod list
		case 'MODS':
			LoadModList(intfc);
			break;

		// Light Mod list
		case 'LMOD':
			LoadLightModList(intfc);
			break;

		// Menu open/close events
		case 'MENR':
			_MESSAGE("Loading menu open/close event registrations...");
			g_menuOpenCloseRegs.Load(intfc, 1);
			break;

		// Key input events
		case 'KEYR':
			_MESSAGE("Loading key input event registrations...");
			g_inputKeyEventRegs.Load(intfc, 1);
			break;

		// Control input events
		case 'CTLR':
			_MESSAGE("Loading control input event registrations...");
			g_inputControlEventRegs.Load(intfc, 1);
			break;

		// Custom mod events
		case 'MCBR':
			_MESSAGE("Loading mod callback event registrations...");
			g_modCallbackRegs.Load(intfc, 1);
			break;

		// Crosshair rev events
		case 'CHRR':
			_MESSAGE("Loading crosshair ref event registrations...");
			g_crosshairRefEventRegs.Load(intfc, 1);
			break;

		// Camera events
		case 'CAMR':
			_MESSAGE("Loading camera event registrations...");
			g_cameraEventRegs.Load(intfc, 1);
			break;

		// Actor Actions events
		case 'AACT':
			_MESSAGE("Loading actor action event registrations...");
			g_actionEventRegs.Load(intfc, 1);
			break;

		// NiNode update events
		case 'NINU':
			_MESSAGE("Loading NiNode update event registrations...");
			g_ninodeUpdateEventRegs.Load(intfc, 1);
			break;

		// SKSEPersistentObjectStorage
		case 'OBMG':
			_MESSAGE("Loading SKSEPersistentObjectStorage data...");
			SKSEObjectStorageInstance().Load(intfc, version);
			break;

		// SKSEDelayFunctorManager
		case 'DFMG':
			_MESSAGE("Loading SKSEDelayFunctorManager data...");
			SKSEDelayFunctorManagerInstance().Load(intfc, version);
			break;

		default:
			_MESSAGE("Unhandled chunk type in Core_LoadCallback: %08X (%.4s)", type, &type);
			continue;
		}
	}
}

void Init_CoreSerialization_Callbacks()
{
	Serialization::SetUniqueID(0, 0);
	Serialization::SetRevertCallback(0, Core_RevertCallback);
	Serialization::SetSaveCallback(0, Core_SaveCallback);
	Serialization::SetLoadCallback(0, Core_LoadCallback);
}
