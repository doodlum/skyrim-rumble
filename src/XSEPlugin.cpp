#include "RumbleManager.h"


extern "C" __declspec(dllexport) const char* NAME = "Rumble";

extern "C" __declspec(dllexport) const char* DESCRIPTION = "by doodlez\n";

static void DrawMenu(reshade::api::effect_runtime*)
{
	RumbleManager::GetSingleton()->Menu();
}


HMODULE     hModuleBackup;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		hModuleBackup = hModule;
		break;
	}
	return TRUE;
}

static void MessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		{
			RumbleManager::GetSingleton()->DataLoaded();
			if (!reshade::register_addon(hModuleBackup)) {
				logger::info("Failed to register addon");
			} else {
				logger::info("Registered addon");
			}
			reshade::register_overlay(nullptr, &DrawMenu);
			break;
		}
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		AnimationEventHandler::Register();
		break;
	}

}

void Load()
{
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener("SKSE", MessageHandler);
	RumbleManager::InstallHooks();
}
