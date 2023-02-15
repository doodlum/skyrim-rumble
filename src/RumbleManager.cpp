#include "RumbleManager.h"

#define MAGIC_ENUM_RANGE_MAX 256
#include "FormUtil.h"
#include <magic_enum.hpp>

#include <SimpleIni.h>

void RumbleManager::AddFoley()
{
	eventVibrations.insert({ animationEventName, {} });
}

void RumbleManager::AddOverride()
{
	if (auto form = RE::TESForm::LookupByEditorID(overrideIdentifier)) {
		if (auto sd = form->As<RE::BGSStandardSoundDef>()) {
			vanillaOriginal.insert({ overrideIdentifier, sd->lengthCharacteristics.rumbleSendValue });
		}
	}
	vanillaOverrides.insert({ overrideIdentifier, { "", 0 } });
}

void RumbleManager::Menu()
{
	static bool showMenu = false;
	ImGui::Checkbox("Show Menu", &showMenu);
	if (showMenu) {
		if (ImGui::Begin("Rumble Config Menu", &showMenu)) {
			if (ImGui::Button("Save")) {
				Save();
			}
			ImGui::SameLine();
			if (ImGui::Button("Load")) {
				Load();
			}
			if (ImGui::BeginTabBar("##")) {
				if (ImGui::BeginTabItem("Main")) {
					ImGui::Checkbox("Enable Mod", &enableMod);
					ImGui::SameLine();
					ImGui::Checkbox("Live Update Overrides", &liveUpdate);

					ImGui::Text("Misc");

					ImGui::InputFloat("Noise Speed", &noiseSpeed);
					ImGui::InputFloat("Quad Sprint Multiplier", &quadSprintMult);

					ImGui::Text("Weapons");

					ImGui::InputFloat("Power Attack Pow", &powerAttackPow);
					ImGui::InputFloat("Swing Weapon Mult", &swingWeaponMult);

					ImGui::Text("Crossbows");
					ImGui::SameLine();
					ImGui::Checkbox("Crossbow Fix", &crossbowFix);
					ImGui::InputFloat("Crossbow Left Motor Speed", &crossbowLeftMotor);
					ImGui::InputFloat("Crossbow Right Motor Speed", &crossbowRightMotor);
					ImGui::InputFloat("Crossbow Duration", &crossbowDuration);

					ImGui::Text("Submerged");

					ImGui::InputFloat("Submerged Pow", &submergedPow);
					ImGui::InputFloat("Submerged Left", &submergedLeftPower);
					ImGui::InputFloat("Submerged Right", &submergedRightPower);
					ImGui::InputFloat("Submerged Mount Multiplier", &submergedMountMult);

					ImGui::Text("Rain");

					ImGui::InputFloat("Rain Pow", &rainPow);
					ImGui::InputFloat("Rain Left", &rainLeftPower);
					ImGui::InputFloat("Rain Right", &rainRightPower);

					ImGui::Text("Grindstones");

					ImGui::InputFloat("Grindstone Pow", &grindstonePow);
					ImGui::InputFloat("Grindstone Left", &grindstoneLeftPower);
					ImGui::InputFloat("Grindstone Right", &grindstoneRightPower);

					ImGui::Text("XAudio DSP");

					ImGui::InputInt("Base Rumble Override", &baseRumbleOverride);

					ImGui::InputFloat("Small Amp Pre Mult", &smallAmpPreMult);
					ImGui::InputFloat("Small Amp Pow", &smallAmpPow);
					ImGui::InputFloat("Small Amp Post Mult", &smallAmpPostMult);
					ImGui::InputFloat("Small Amp Left Balance", &smallAmpLeftBalance);
					ImGui::InputFloat("Small Amp Right Balance", &smallAmpRightBalance);

					ImGui::InputFloat("Large Amp Pre Mult", &largeAmpPreMult);
					ImGui::InputFloat("Large Amp Pow", &largeAmpPow);
					ImGui::InputFloat("Large Amp Post Mult", &largeAmpPostMult);
					ImGui::InputFloat("Large Amp Left Balance", &largeAmpLeftBalance);
					ImGui::InputFloat("Large Amp Right Balance", &largeAmpRightBalance);

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Custom Sources")) {
					for (int i = 0; i < VibrationsCustom::VibrationsCustomCount; i++) {
						ImGui::Text(std::format("Source: {}", magic_enum::enum_name((VibrationsCustom)i)).c_str());
						ImGui::InputFloat(std::format("{} Left Motor Speed", i).c_str(), &sourcesCustom[(VibrationsCustom)i].motors.x);
						ImGui::InputFloat(std::format("{} Right Motor Speed", i).c_str(), &sourcesCustom[(VibrationsCustom)i].motors.y);
						ImGui::InputFloat(std::format("{} Duration", i).c_str(), &sourcesCustom[(VibrationsCustom)i].duration);
						ImGui::InputInt(std::format("{} Type", i).c_str(), (int*)&(sourcesCustom[(VibrationsCustom)i].type));
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Events")) {
					int  i = 0;
					auto itr = eventVibrations.begin();
					while (itr != eventVibrations.end()) {
						if (ImGui::Button(std::format("{}F Delete", i).c_str())) {
							eventVibrations.erase(itr++);
						} else {
							auto& custom = *itr;
							ImGui::Text(std::format("Event Source: {}", custom.first).c_str());
							ImGui::InputFloat(std::format("{}E Left Motor Speed", i).c_str(), &custom.second.motors.x);
							ImGui::InputFloat(std::format("{}E Right Motor Speed", i).c_str(), &custom.second.motors.y);
							ImGui::InputFloat(std::format("{}E Duration", i).c_str(), &custom.second.duration);
							ImGui::InputInt(std::format("{}E Type", i).c_str(), (int*)&(custom.second.type));
							i++;
							++itr;
						}
					}
					ImGui::Text("");
					ImGui::InputText("Event Name", &animationEventName);
					if (ImGui::Button("Add New Event")) {
						AddFoley();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Overrides")) {
					int  i = 0;
					auto itr = vanillaOverrides.begin();
					while (itr != vanillaOverrides.end()) {
						if (ImGui::Button(std::format("{}F Delete", i).c_str())) {
							vanillaOverrides.erase(itr++);
						} else {
							auto& custom = *itr;
							ImGui::Text(std::format("SD Form Identifier: {}", custom.first).c_str());
							ImGui::InputText(std::format("{}SD Tag", custom.first).c_str(), &custom.second.tag);
							ImGui::InputInt(std::format("{}SD Send Rumble Value", i).c_str(), &custom.second.rumbleSendValue);
							i++;
							++itr;
						}
					}
					ImGui::Text("");
					ImGui::InputText("Form Identifier", &overrideIdentifier);
					if (ImGui::Button("Add New Event")) {
						AddOverride();
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
	}
}

std::string capitalize(std::string s)
{
	bool cap = true;
	for (unsigned int i = 0; i <= s.length(); i++) {
		if (isalpha(s[i]) && cap == true) {
			s[i] = (char)toupper(s[i]);
			cap = false;
		} else if (isspace(s[i])) {
			cap = true;
		}
	}
	return s;
}

#define GetSettingFloat(a_section, a_setting) a_setting = (float)ini.GetDoubleValue(a_section, capitalize(#a_setting).c_str(), a_setting);
#define SetSettingFloat(a_section, a_setting) ini.SetDoubleValue(a_section, capitalize(#a_setting).c_str(), a_setting);

#define GetSettingInt(a_section, a_setting) a_setting = std::stoi(ini.GetValue(a_section, capitalize(#a_setting).c_str(), std::to_string(a_setting).c_str()));
#define SetSettingInt(a_section, a_setting) ini.SetValue(a_section, capitalize(#a_setting).c_str(), std::to_string(a_setting).c_str());

#define GetSettingBool(a_section, a_setting) a_setting = ini.GetBoolValue(a_section, capitalize(#a_setting).c_str(), a_setting);
#define SetSettingBool(a_section, a_setting) ini.SetBoolValue(a_section, capitalize(#a_setting).c_str(), a_setting);

void RumbleManager::Load()
{
	try {
		{
			std::ifstream i(L"Data\\SKSE\\Plugins\\Rumble.json");
			json          settings;
			i >> settings;
			eventVibrations.clear();
			if (settings.contains("Events")) {
				for (auto& entry : settings["Events"]) {
					VibrationCustom source = Create(entry["Duration"], entry["Left"], entry["Right"], entry["Type"]);
					eventVibrations.insert({ entry["Event"], source });
				}
			}
			vanillaOverrides.clear();
			if (settings.contains("Overrides")) {
				for (auto& entry : settings["Overrides"]) {
					if (!vanillaOriginal.contains(entry["Form Identifier"])) {
						if (auto form = RE::TESForm::LookupByEditorID(entry["Form Identifier"])) {
							if (auto sd = form->As<RE::BGSStandardSoundDef>()) {
								vanillaOriginal.insert({ entry["Form Identifier"], sd->lengthCharacteristics.rumbleSendValue });
							}
						}
					}
					VanillaOverride e{ entry["Tag"], entry["Send Rumble Value"] };
					vanillaOverrides.insert({ entry["Form Identifier"], e });
				}
			}
		}
		{
			std::lock_guard<std::shared_mutex> lk(mutex);
			CSimpleIniA                        ini;
			ini.SetUnicode();
			ini.LoadFile(L"Data\\SKSE\\Plugins\\Rumble.ini");

			GetSettingFloat("Misc", noiseSpeed);
			GetSettingFloat("Misc", quadSprintMult);

			GetSettingFloat("Weapons", powerAttackPow);
			GetSettingFloat("Weapons", swingWeaponMult);

			GetSettingBool("Crossbows", crossbowFix);
			GetSettingFloat("Crossbows", crossbowDuration);
			GetSettingFloat("Crossbows", crossbowLeftMotor);
			GetSettingFloat("Crossbows", crossbowRightMotor);

			GetSettingFloat("Submerged", submergedPow);
			GetSettingFloat("Submerged", submergedLeftPower);
			GetSettingFloat("Submerged", submergedRightPower);
			GetSettingFloat("Submerged", submergedMountMult);

			GetSettingFloat("Rain", rainPow);
			GetSettingFloat("Rain", rainLeftPower);
			GetSettingFloat("Rain", rainRightPower);

			GetSettingFloat("Grindstones", grindstonePow);
			GetSettingFloat("Grindstones", grindstoneLeftPower);
			GetSettingFloat("Grindstones", grindstoneRightPower);

			GetSettingInt("XAudioDSP", baseRumbleOverride);

			GetSettingFloat("XAudioDSP", smallAmpPreMult);
			GetSettingFloat("XAudioDSP", smallAmpPow);
			GetSettingFloat("XAudioDSP", smallAmpPostMult);
			GetSettingFloat("XAudioDSP", smallAmpLeftBalance);
			GetSettingFloat("XAudioDSP", smallAmpRightBalance);

			GetSettingFloat("XAudioDSP", largeAmpPreMult);
			GetSettingFloat("XAudioDSP", largeAmpPow);
			GetSettingFloat("XAudioDSP", largeAmpPostMult);
			GetSettingFloat("XAudioDSP", largeAmpLeftBalance);
			GetSettingFloat("XAudioDSP", largeAmpRightBalance);

			for (int i = 0; i < VibrationsCustom::VibrationsCustomCount; i++) {
				VibrationCustom& source = sourcesCustom[(VibrationsCustom)i];
				auto             name = std::format("{}", magic_enum::enum_name((VibrationsCustom)i)).c_str();
				source.motors.x = (float)ini.GetDoubleValue(name, "LeftMotor", source.motors.x);
				source.motors.y = (float)ini.GetDoubleValue(name, "RightMotor", source.motors.y);
				source.duration = (float)ini.GetDoubleValue(name, "Duration", source.duration);
				source.type = (VibrationType)std::stoi(ini.GetValue(name, "Type", std::to_string(source.type).c_str()));
			}
		}
	} catch (...) {
	}
}

void RumbleManager::Save()
{
	try {
		{
			std::ofstream o(L"Data\\SKSE\\Plugins\\Rumble.json");
			json          settings;
			json          events;
			for (auto& entry : eventVibrations) {
				json root;
				root["Event"] = entry.first;
				root["Duration"] = entry.second.duration;
				root["Left"] = entry.second.motors.x;
				root["Right"] = entry.second.motors.y;
				root["Type"] = entry.second.type;
				events.emplace_back(root);
			}
			settings["Events"] = events;
			json overrides;
			for (auto& entry : vanillaOverrides) {
				json root;
				root["Form Identifier"] = entry.first;
				root["Tag"] = entry.second.tag;
				root["Send Rumble Value"] = entry.second.rumbleSendValue;
				overrides.emplace_back(root);
			}
			settings["Overrides"] = overrides;
			o << settings.dump(1);
		}
		{
			std::lock_guard<std::shared_mutex> lk(mutex);
			CSimpleIniA                        ini;
			ini.SetUnicode();

			SetSettingFloat("Misc", noiseSpeed);
			SetSettingFloat("Misc", quadSprintMult);

			SetSettingFloat("Weapons", powerAttackPow);
			SetSettingFloat("Weapons", swingWeaponMult);

			SetSettingBool("Crossbows", crossbowFix);
			SetSettingFloat("Crossbows", crossbowDuration);
			SetSettingFloat("Crossbows", crossbowLeftMotor);
			SetSettingFloat("Crossbows", crossbowRightMotor);

			SetSettingFloat("Submerged", submergedPow);
			SetSettingFloat("Submerged", submergedLeftPower);
			SetSettingFloat("Submerged", submergedRightPower);
			SetSettingFloat("Submerged", submergedMountMult);

			SetSettingFloat("Rain", rainPow);
			SetSettingFloat("Rain", rainLeftPower);
			SetSettingFloat("Rain", rainRightPower);

			SetSettingFloat("Grindstones", grindstonePow);
			SetSettingFloat("Grindstones", grindstoneLeftPower);
			SetSettingFloat("Grindstones", grindstoneRightPower);

			SetSettingInt("XAudioDSP", baseRumbleOverride);

			SetSettingFloat("XAudioDSP", smallAmpPreMult);
			SetSettingFloat("XAudioDSP", smallAmpPow);
			SetSettingFloat("XAudioDSP", smallAmpPostMult);
			SetSettingFloat("XAudioDSP", smallAmpLeftBalance);
			SetSettingFloat("XAudioDSP", smallAmpRightBalance);

			SetSettingFloat("XAudioDSP", largeAmpPreMult);
			SetSettingFloat("XAudioDSP", largeAmpPow);
			SetSettingFloat("XAudioDSP", largeAmpPostMult);
			SetSettingFloat("XAudioDSP", largeAmpLeftBalance);
			SetSettingFloat("XAudioDSP", largeAmpRightBalance);

			for (int i = 0; i < VibrationsCustom::VibrationsCustomCount; i++) {
				VibrationCustom& source = sourcesCustom[(VibrationsCustom)i];
				auto             name = std::format("{}", magic_enum::enum_name((VibrationsCustom)i));
				ini.SetDoubleValue(name.c_str(), "LeftMotor", source.motors.x);
				ini.SetDoubleValue(name.c_str(), "RightMotor", source.motors.y);
				ini.SetDoubleValue(name.c_str(), "Duration", source.duration);
				ini.SetValue(name.c_str(), "Type", std::to_string(source.type).c_str());
			}

			ini.SaveFile(L"Data\\SKSE\\Plugins\\Rumble.ini");
		}
	} catch (...) {
	}
}

void RumbleManager::VanillaOverrideBase()
{
	for (auto& ptr : overrideForms) {
		RE::BGSStandardSoundDef* soundDef = (RE::BGSStandardSoundDef*)ptr;
		soundDef->lengthCharacteristics.rumbleSendValue = (std::uint8_t)baseRumbleOverride;
	}
	for (auto& ptr : overrideCrossbows) {
		RE::TESObjectWEAP::RangedData* rangedData = (RE::TESObjectWEAP::RangedData*)ptr;
		rangedData->firingRumbleDuration = crossbowFix ? crossbowDuration : 0;
		rangedData->firingRumbleLeftMotorStrength = crossbowLeftMotor;
		rangedData->firingRumbleRightMotorStrength = crossbowRightMotor;
	}
}

void RumbleManager::UpdateOverrides()
{
	VanillaOverrideBase();
	for (auto& entry : vanillaOverrides) {
		if (auto form = FormUtil::GetFormFromIdentifier(entry.first)) {
			if (auto formTyped = form->As<RE::BGSSoundDescriptorForm>()) {
				RE::BGSStandardSoundDef* soundDef = (RE::BGSStandardSoundDef*)formTyped->soundDescriptor;
				soundDef->lengthCharacteristics.rumbleSendValue = (std::uint8_t)entry.second.rumbleSendValue;
			}
		}
	}
}

void RumbleManager::DataLoaded()
{
	auto  dataHandler = RE::TESDataHandler::GetSingleton();
	auto& soundDescriptorArray = dataHandler->GetFormArray<RE::BGSSoundDescriptorForm>();
	for (auto& entry : soundDescriptorArray) {
		RE::BGSStandardSoundDef* sd = (RE::BGSStandardSoundDef*)entry->soundDescriptor;
		if (sd->lengthCharacteristics.rumbleSendValue == 0) {
			overrideForms.insert(sd);
		}
		if (sd->lengthCharacteristics.rumbleSendValue == 255) {
			sd->lengthCharacteristics.rumbleSendValue = 0;
		}
	}
	auto& weaponArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();
	for (auto& entry : weaponArray) {
		if (entry->IsCrossbow() && entry->weaponData.rangedData && entry->weaponData.rangedData && entry->weaponData.rangedData->firingRumbleDuration == 0) {
			overrideCrossbows.insert(entry->weaponData.rangedData);
		}
	}
	FootstepEventHandler::Register();
	MenuOpenCloseEventHandler::Register();
	UpdateOverrides();
	dataLoaded = true;
}

RE::BSEventNotifyControl FootstepEventHandler::ProcessEvent(const RE::BGSFootstepEvent* a_event, RE::BSTEventSource<RE::BGSFootstepEvent>*)
{
	RumbleManager::GetSingleton()->FootstepEvent(a_event);
	return RE::BSEventNotifyControl::kContinue;
}

[[nodiscard]] static RE::BGSFootstepManager* GetFootstepManager()
{
	REL::Relocation<RE::BGSFootstepManager**> singleton{ RELOCATION_ID(517045, 403553) };
	return *singleton;
}

bool FootstepEventHandler::Register()
{
	static FootstepEventHandler singleton;
	GetFootstepManager()->AddEventSink(&singleton);
	logger::info("Registered {}", typeid(singleton).name());
	return true;
}

void RumbleManager::FootstepEvent(const RE::BGSFootstepEvent* a_event)
{
	if (auto player = RE::PlayerCharacter::GetSingleton()) {
		RE::ActorPtr mount;
		if (player->GetMount(mount) && mount) {
			if (a_event->actor == mount->GetHandle()) {
				if (a_event->tag == "FootFront") {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepFront];
					if (mount->AsActorState()->IsSprinting()) {
						temp.motors *= quadSprintMult;
					}
					Trigger(temp);
				} else {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepBack];
					if (mount->AsActorState()->IsSprinting()) {
						temp.motors *= quadSprintMult;
					}
					Trigger(temp);
				}
			}
		} else if (a_event->actor == player->GetHandle()) {
			if (a_event->tag == "FootSprintLeft") {
				VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepSprintLeft];
				Trigger(temp);
			} else if (a_event->tag == "FootSprintRight") {
				VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepSprintRight];
				Trigger(temp);
			} else {
				float level = GetSubmergedLevel(player);
				if (level > 0.0f) {
					if (a_event->tag == "FootLeft") {
						VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepWadingLeft];
						temp.motors = (temp.motors * 0.25f) + temp.motors * level * 0.75f;
						Trigger(temp);
					} else if (a_event->tag == "FootRight") {
						VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepWadingRight];
						temp.motors = (temp.motors * 0.25f) + temp.motors * level * 0.75f;
						Trigger(temp);
					} else {
						VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepWadingLeft];
						temp.motors = (temp.motors * 0.25f) + temp.motors * level * 0.75f;
						Trigger(temp);
						temp = sourcesCustom[VibrationsCustom::FootstepWadingRight];
						temp.motors = (temp.motors * 0.25f) + temp.motors * level * 0.75f;
						Trigger(temp);
					}
				} else if (a_event->tag == "FootFront") {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepFront];
					if (player->AsActorState()->IsSprinting()) {
						temp.motors *= quadSprintMult;
					}
					Trigger(temp);
				} else if (a_event->tag == "FootBack") {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepBack];
					if (player->AsActorState()->IsSprinting()) {
						temp.motors *= quadSprintMult;
					}
					Trigger(temp);
				}
			}
		}
	}
}

RE::BSEventNotifyControl AnimationEventHandler::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
{
	RumbleManager::GetSingleton()->AnimationEvent(a_event);
	return RE::BSEventNotifyControl::kContinue;
}

bool AnimationEventHandler::Register()
{
	static AnimationEventHandler singleton;
	RE::PlayerCharacter::GetSingleton()->AddAnimationGraphEventSink(&singleton);
	logger::info("Registered {}", typeid(singleton).name());
	return true;
}

void RumbleManager::AnimationEvent(const RE::BSAnimationGraphEvent* a_event)
{
	std::string stringified = a_event->tag.c_str();
	auto        it = eventVibrations.find(stringified);
	if (it != eventVibrations.end()) {
		Trigger((*it).second);
	} else if (stringified == "tailSharpeningWheel") {
		onGrindstone = true;
	} else if (stringified == "FootSprintLeft") {
		VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepSprintLeft];
		Trigger(temp);
	} else if (stringified == "FootSprintRight") {
		VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepSprintRight];
		Trigger(temp);
	} else {
		onGrindstone = false;
	}
}

RE::BSEventNotifyControl MenuOpenCloseEventHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	if (a_event->menuName == RE::LoadingMenu::MENU_NAME) {
		std::lock_guard<std::shared_mutex> lk(RumbleManager::GetSingleton()->mutex);
		RumbleManager::GetSingleton()->activeVibrations.clear();
		RumbleManager::GetSingleton()->inLoadingScreen = a_event->opening;
	}
	return RE::BSEventNotifyControl::kContinue;
}

bool MenuOpenCloseEventHandler::Register()
{
	static MenuOpenCloseEventHandler singleton;
	RE::UI::GetSingleton()->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(&singleton);
	logger::info("Registered {}", typeid(singleton).name());
	return true;
}

bool RumbleManager::PlayerHasCrossbow()
{
	if (auto player = RE::PlayerCharacter::GetSingleton()) {
		if (auto object = player->GetEquippedObject(false)) {
			if (auto weapon = object->As<RE::TESObjectWEAP>()) {
				return weapon->IsCrossbow();
			}
		}
	}
	return false;
}

bool RumbleManager::ProcessHit(int type, float power, float)
{
	if (type == 1) {
		return !PlayerHasCrossbow();
	} else if (power == 0.5) {
		return !PlayerHasCrossbow();
	}
	return true;
}

void RumbleManager::DisableFeatures()
{
	for (auto& ptr : overrideForms) {
		RE::BGSStandardSoundDef* soundDef = (RE::BGSStandardSoundDef*)ptr;
		soundDef->lengthCharacteristics.rumbleSendValue = 0;
	}
	for (auto& ptr : overrideCrossbows) {
		RE::TESObjectWEAP::RangedData* rangedData = (RE::TESObjectWEAP::RangedData*)ptr;
		rangedData->firingRumbleDuration = 0;
	}
}

void RumbleManager::SetState(XINPUT_VIBRATION* pVibration)
{
	if (liveUpdate && dataLoaded) {
		UpdateOverrides();
	}
	if (!inLoadingScreen) {
		if (auto ui = RE::UI::GetSingleton()) {
			if (!ui->GameIsPaused()) {
				std::lock_guard<std::shared_mutex> lk(mutex);

				double leftVibration = 0;
				double rightVibration = 0;

				static float& deltaTime = (*(float*)REL::RelocationID(523660, 410199).address());  // 2F6B948, 30064C8

				std::list<VibrationCustom>::iterator itr = activeVibrations.begin();
				while (itr != activeVibrations.end()) {
					auto& source = *itr;
					if (source.time <= 0) {
						activeVibrations.erase(itr++);
						continue;
					} else if (source.time <= source.duration) {
						double strength = 1.0;
						if (source.type == VibrationType::kSmooth) {
							strength = source.time / (source.duration + FLT_MIN);
						} else if (source.type == VibrationType::kBump) {
							strength = SmoothBumpStep(0.0, (double)source.duration, (double)source.time);
						}
						leftVibration += source.motors.x * strength;
						rightVibration += source.motors.y * strength;
					}
					source.time = source.time - deltaTime;
					++itr;
				}

				if (onGrindstone) {
					double noise = pow(noise1.noise1D_01(noise1timer), grindstonePow);
					leftVibration = max(leftVibration, noise * grindstoneLeftPower);
					rightVibration = max(rightVibration, noise * grindstoneRightPower);
				} else if (auto player = RE::PlayerCharacter::GetSingleton()) {
					float        level = GetSubmergedLevel(player);
					RE::ActorPtr mount;
					float        boost = 1.0f;
					if (player->GetMount(mount) && mount) {
						level = GetSubmergedLevel(mount.get());
						boost = submergedMountMult;
					}
					if (level > 0) {
						double noise = pow(noise1.noise1D_01(noise1timer), submergedPow) * level * boost;
						leftVibration = max(leftVibration, noise * submergedLeftPower);
						rightVibration = max(rightVibration, noise * submergedRightPower);
					}
					if (auto sky = RE::Sky::GetSingleton()) {
						if (auto cell = player->GetParentCell()) {
							if (!cell->IsInteriorCell()) {
								if (sky->IsRaining()) {
									double noise = pow(noise1.noise1D_01(noise1timer), rainPow);
									leftVibration = std::lerp(max(leftVibration, noise * rainLeftPower), leftVibration, (double)level);
									rightVibration = std::lerp(max(rightVibration, noise * rainRightPower), rightVibration, (double)level);
								}
							}
						}
					}
				}

				double powMult = 1.0f;

				if (auto player = RE::PlayerCharacter::GetSingleton()) {
					if (IsPowerAttacking(player)) {
						powMult *= powerAttackPow;
					}
					if (auto weapon = player->GetAttackingWeapon()) {
						if (auto object = weapon->object) {
							powMult *= swingWeaponMult;
						}
					}
				}

				double smallAmp = (pow(1 + (smallRumble * smallAmpPreMult), smallAmpPow * powMult) - 1) * smallAmpPostMult;
				double largeAmp = (pow(1 + (largeRumble * largeAmpPreMult), largeAmpPow * powMult) - 1) * largeAmpPostMult;

				if (auto player = RE::PlayerCharacter::GetSingleton()) {
					if (auto weapon = player->GetAttackingWeapon()) {
						if (auto object = weapon->object) {
							smallAmp *= swingWeaponMult;
							largeAmp *= swingWeaponMult;
						}
					}
				}

				smallAmp = std::clamp(smallAmp, 0.0, 1.0);
				largeAmp = std::clamp(largeAmp, 0.0, 1.0);

				double leftAdd = std::lerp(smallAmp * smallAmpLeftBalance, smallAmp, smallAmp) + std::lerp(largeAmp, largeAmp * largeAmpRightBalance, largeAmp);
				double rightAdd = std::lerp(smallAmp, smallAmp * smallAmpRightBalance, smallAmp) + std::lerp(largeAmp * largeAmpLeftBalance, largeAmp, largeAmp);

				double leftVibrationOriginal = (double)pVibration->wLeftMotorSpeed / 65535;
				double rightVibrationOriginal = (double)pVibration->wRightMotorSpeed / 65535;

				leftVibration += leftAdd;
				rightVibration += rightAdd;

				leftVibrationOriginal += leftAdd;
				rightVibrationOriginal += rightAdd;

				leftVibration = min(leftVibration, 1.0f);
				rightVibration = min(rightVibration, 1.0f);

				leftVibrationOriginal = min(leftVibrationOriginal, 1.0f);
				rightVibrationOriginal = min(rightVibrationOriginal, 1.0f);

				pVibration->wLeftMotorSpeed = max(static_cast<std::uint16_t>(leftVibrationOriginal * 65535.0), static_cast<std::uint16_t>(leftVibration * 65535.0));
				pVibration->wRightMotorSpeed = max(static_cast<std::uint16_t>(rightVibrationOriginal * 65535.0), static_cast<std::uint16_t>(rightVibration * 65535.0));

				noise1timer += deltaTime * noiseSpeed;

				if (noise1timer == DBL_MAX) {
					noise1timer = 0;
				}
			} else {
				long double leftVibrationOriginal = (double)pVibration->wLeftMotorSpeed / 65535;
				long double rightVibrationOriginal = (double)pVibration->wRightMotorSpeed / 65535;

				double smallAmp = (pow(1 + (smallRumble * smallAmpPreMult), smallAmpPow) - 1) * smallAmpPostMult;
				double largeAmp = (pow(1 + (largeRumble * largeAmpPreMult), largeAmpPow) - 1) * largeAmpPostMult;

				smallAmp = std::clamp(smallAmp, 0.0, 1.0);
				largeAmp = std::clamp(largeAmp, 0.0, 1.0);

				double leftAdd = std::lerp(smallAmp * smallAmpLeftBalance, smallAmp, smallAmp) + std::lerp(largeAmp, largeAmp * largeAmpRightBalance, largeAmp);
				double rightAdd = std::lerp(smallAmp, smallAmp * smallAmpRightBalance, smallAmp) + std::lerp(largeAmp * largeAmpLeftBalance, largeAmp, largeAmp);

				leftVibrationOriginal += leftAdd;
				rightVibrationOriginal += rightAdd;

				leftVibrationOriginal = min(leftVibrationOriginal, 1.0f);
				rightVibrationOriginal = min(rightVibrationOriginal, 1.0f);

				pVibration->wLeftMotorSpeed = static_cast<std::uint16_t>(leftVibrationOriginal * 65535.0);
				pVibration->wRightMotorSpeed = static_cast<std::uint16_t>(rightVibrationOriginal * 65535.0);
			}
		}
	} else {
		pVibration->wLeftMotorSpeed = 0;
		pVibration->wRightMotorSpeed = 0;
	}
}
