#include "common/IDebugLog.h"  // IDebugLog
#include "skse64_common/skse_version.h"  // RUNTIME_VERSION
#include "skse64/PluginAPI.h"  // SKSEInterface, PluginInfo

#include <ShlObj.h>  // CSIDL_MYDOCUMENTS

#include "SKSE/API.h"
#include "SKSE/Interfaces.h"
#include "skse64/GameData.h"

#include "patches.h"
#include "fixes.h"

#include "version.h"  // VERSION_VERSTRING, VERSION_MAJOR

void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
		{
		_MESSAGE("Message kDataLoaded");
		DataHandler* dh = DataHandler::GetSingleton();

		_MESSAGE("dh = %016I64X", dh);
		_MESSAGE("dh->modList.loadedMods = %016I64X", &dh->modList.loadedModCount);

		_VMESSAGE("Loaded Mods = %x", dh->modList.loadedModCount);
		}
		break;
	default:
		break;
	}
	return;
}


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		SKSE::Logger::OpenRelative(FOLDERID_Documents, R"(\\My Games\\Skyrim VR\\SKSE\\EngineFixesVR.log)");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::TrackTrampolineStats(true);
		SKSE::Logger::UseLogStamp(true);


		_MESSAGE("EngineFixes v%s", EFVR_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "EngineFixesVR";
		a_info->version = EFVR_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}

		auto ver = a_skse->RuntimeVersion();
		if (ver <= SKSE::RUNTIME_VR_1_4_15)
		{
			_FATALERROR("Unsupported runtime version %s!\n", ver.GetString().c_str());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		if (!SKSE::Init(a_skse))
		{
			return false;
		}

		if (!SKSE::AllocTrampoline(1 << 11))
		{
			return false;
		}

		//Sleep(10000);
		auto messaging = SKSE::GetMessagingInterface();

		if (!messaging->RegisterListener("SKSE", MessageHandler))
		{
			return false;
		}

		if (config::verboseLogging)
		{
			_MESSAGE("enabling verbose logging");
			SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kVerboseMessage);
			SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kVerboseMessage);
		}

		std::uintptr_t base = REL::Module::BaseAddr();
		_MESSAGE("baseaddr = %016I64X", base);

		REL::Offset<std::uint32_t*> testit(base + 0x194230);     // SSE has this offset as 0x194230
		_MESSAGE("testit = %016I64X", testit.GetAddress());
		_MESSAGE("testit = %016I64X", testit.GetOffset());

		patches::PatchAll();

		fixes::PatchAll();

		_MESSAGE("[MESSAGE] EngineFixes loaded");
		return true;
	}
};
