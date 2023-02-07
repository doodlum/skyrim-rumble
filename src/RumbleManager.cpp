#include "RumbleManager.h"

#define MAGIC_ENUM_RANGE_MAX 256
#include "FormUtil.h"
#include <magic_enum.hpp>

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
	if (ImGui::Begin("RERumble Config Menu", &showMenu)) {
		if (ImGui::Button("Save JSON")) {
			SaveJSON();
		}
		if (ImGui::Button("Load JSON")) {
			LoadJSON();
		}
		if (ImGui::BeginTabBar("##")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::Selectable("Live Update Overrides", &liveUpdate);

				ImGui::Text("Misc");

				ImGui::InputFloat("Noise 1 Speed", &noise1speed);
				ImGui::InputFloat("Quad Sprint Multiplier", &quadSprintMult);

				ImGui::Text("Weapons");

				ImGui::InputFloat("Power Attack Pow", &powerAttackPow);
				ImGui::InputFloat("Swing Weapon Mult", &swingWeaponMult);

				ImGui::Text("Submerged");

				ImGui::InputFloat("Submerged Pow", &submergedPow);
				ImGui::InputFloat("Submerged Small", &submergedSmallPower);
				ImGui::InputFloat("Submerged Large", &submergedLargePower);
				ImGui::InputFloat("Submerged Mount Multiplier", &submergedMountMult);

				ImGui::Text("Rain");

				ImGui::InputFloat("Rain Pow", &rainPow);
				ImGui::InputFloat("Rain Small", &rainSmallPower);
				ImGui::InputFloat("Rain Large", &rainLargePower);

				ImGui::Text("Grindstones");
				ImGui::InputFloat("Grindstone Pow", &grindstonePow);
				ImGui::InputFloat("Grindstone Small", &grindstoneSmallPower);
				ImGui::InputFloat("Grindstone Large", &grindstoneLargePower);

				ImGui::Text("Audio DSP");

				ImGui::InputInt("Base Rumble Override", &baseRumbleOverride);

				ImGui::InputFloat("Small Amp Pre Mult", &smallAmpPreMult);
				ImGui::InputFloat("Small Amp Pow", &smallAmpPow);
				ImGui::InputFloat("Small Amp Post Mult", &smallAmpPostMult);
				ImGui::InputFloat("Small Amp Left Balance", &smallAmpLeftBalance);
				ImGui::InputFloat("Small Amp Right Balance", &smallAmpRightBalance);

				ImGui::InputFloat("Large Amp Pre Mult", &largeAmpPreMult);
				ImGui::InputFloat("Large Amp Pow", &largeAmpPow);
				ImGui::InputFloat("Large Amp Post Mult", &largeAmpPostMult);

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

void RumbleManager::LoadJSON()
{
	try {
		std::ifstream i(L"Data\\SKSE\\Plugins\\RERumble.json");
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
	} catch (...) {
	}
}

void RumbleManager::SaveJSON()
{
	try {
		std::ofstream o(L"Data\\SKSE\\Plugins\\RERumble.json");
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
	} catch (...) {
	}
}

void RumbleManager::UpdateOverrides()
{
	VanillaOverrideBase();
	for (auto& entry : vanillaOverrides) {
		if (auto form = FormUtil::GetFormFromIdentifier(entry.first)) {
			if (auto formTyped = form->As<RE::BGSSoundDescriptorForm>()) {
				RE::BGSStandardSoundDef* sd = (RE::BGSStandardSoundDef*)formTyped->soundDescriptor;
				sd->lengthCharacteristics.rumbleSendValue = (std::uint8_t)entry.second.rumbleSendValue;
			}
		}
	}
}

void RumbleManager::VanillaOverrideBase()
{
	for (auto& ptr : overrideForms) {
		RE::BGSStandardSoundDef* sd = (RE::BGSStandardSoundDef*)ptr;
		sd->lengthCharacteristics.rumbleSendValue = (std::uint8_t)baseRumbleOverride;
	}
}

void RumbleManager::DataLoaded()
{
	auto  dataHandler = RE::TESDataHandler::GetSingleton();
	auto& arr = dataHandler->GetFormArray<RE::BGSSoundDescriptorForm>();
	for (auto& entry : arr) {
		RE::BGSStandardSoundDef* sd = (RE::BGSStandardSoundDef*)entry->soundDescriptor;
		if (sd->lengthCharacteristics.rumbleSendValue == 0) {
			overrideForms.insert(sd);
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
						temp.duration *= quadSprintMult;
					}
					Trigger(temp);
				} else {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepBack];
					if (mount->AsActorState()->IsSprinting()) {
						temp.motors *= quadSprintMult;
						temp.duration *= quadSprintMult;
					}
					Trigger(temp);
				}
			}
		} else if (a_event->actor == player->GetHandle()) {
			float level = GetSubmergedLevel(player);
			if (level > 0.0f) {
				if (a_event->tag == "FootLeft") {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepWadingLeft];
					temp.motors = (temp.motors * 0.1f) + temp.motors * level * 0.9f;
					Trigger(temp);
				} else if (a_event->tag == "FootRight") {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepWadingRight];
					temp.motors = (temp.motors * 0.1f) + temp.motors * level * 0.9f;
					Trigger(temp);
				} else {
					VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepWadingLeft];
					temp.motors = (temp.motors * 0.1f) + temp.motors * level * 0.9f;
					Trigger(temp);
					temp = sourcesCustom[VibrationsCustom::FootstepWadingRight];
					temp.motors = (temp.motors * 0.1f) + temp.motors * level * 0.9f;
					Trigger(temp);
				}
			} else if (a_event->tag == "FootFront") {
				VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepFront];
				if (player->AsActorState()->IsSprinting()) {
					temp.motors *= quadSprintMult;
					temp.duration *= quadSprintMult;
				}
				Trigger(temp);
			} else if (a_event->tag == "FootBack") {
				VibrationCustom temp = sourcesCustom[VibrationsCustom::FootstepBack];
				if (player->AsActorState()->IsSprinting()) {
					temp.motors *= quadSprintMult;
					temp.duration *= quadSprintMult;
				}
				Trigger(temp);
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
	}
	if (stringified == "tailSharpeningWheel") {
		onGrindstone = true;
	} else {
		onGrindstone = false;
	}
}

RE::DestructibleObjectData* GetDestructibleForm(RE::TESBoundObject* a_form)
{
	using func_t = decltype(&GetDestructibleForm);
	REL::Relocation<func_t> func{ REL::RelocationID(14055, 14152) };  // 1.5.97 1401832b0
	return func(a_form);
}

RE::BSEventNotifyControl MenuOpenCloseEventHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	if (a_event->menuName == RE::LoadingMenu::MENU_NAME) {
		RumbleManager::GetSingleton()->activeVibrations.clear();
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

bool IsDestructible(RE::TESObjectREFR* a_form)
{
	if (a_form && GetDestructibleForm(a_form->GetBaseObject()))
		return true;
	return false;
}

bool HasVelocity(RE::TESObjectREFR* a_form)
{
	RE::NiPoint3 velocity;
	a_form->GetLinearVelocity(velocity);
	return velocity.Length();
}

bool PlayerHasCrossbow()
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

bool RumbleManager::SetState(XINPUT_VIBRATION* pVibration)
{
	if (liveUpdate && dataLoaded) {
		UpdateOverrides();
	}
	if (auto ui = RE::UI::GetSingleton()) {
		if (!ui->GameIsPaused()) {
			long double leftVibration = 0;
			long double rightVibration = 0;

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
					leftVibration = max(leftVibration, source.motors.x * strength);
					rightVibration = max(rightVibration, source.motors.y * strength);
				}
				source.time = source.time - deltaTime;
				++itr;
			}

			if (onGrindstone) {
				double noise = pow(noise1.noise1D_01(noise1timer), grindstonePow);
				leftVibration = max(leftVibration, noise * grindstoneSmallPower);
				rightVibration = max(rightVibration, noise * grindstoneLargePower);
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
					leftVibration = max(leftVibration, noise * submergedSmallPower);
					rightVibration = max(rightVibration, noise * submergedLargePower);
				}
				if (auto sky = RE::Sky::GetSingleton()) {
					if (auto cell = player->GetParentCell()) {
						if (!cell->IsInteriorCell()) {
							if (sky->IsRaining()) {
								double noise = pow(noise1.noise1D_01(noise1timer), rainPow);
								leftVibration = std::lerp(max(leftVibration, noise * rainSmallPower), leftVibration, (long double)level);
								rightVibration = std::lerp(max(rightVibration, noise * rainLargePower), rightVibration, (long double)level);
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

			if (auto player = RE::PlayerCharacter::GetSingleton()) {
				if (auto weapon = player->GetAttackingWeapon()) {
					if (auto object = weapon->object) {
						smallAmp *= swingWeaponMult;
					}
				}
			}

			smallAmp = std::clamp(smallAmp, 0.0, 1.0);

			auto leftAdd = std::lerp(smallAmp * smallAmpLeftBalance, smallAmp, smallAmp);
			auto rightAdd = std::lerp(smallAmp, smallAmp * smallAmpRightBalance, smallAmp);

			leftVibration += leftAdd + largeAmp;
			rightVibration += rightAdd;

			long double leftVibrationOriginal = (double)pVibration->wLeftMotorSpeed / 65535;
			long double rightVibrationOriginal = (double)pVibration->wRightMotorSpeed / 65535;

			leftVibrationOriginal += leftAdd + largeAmp;
			rightVibrationOriginal += rightAdd;

			leftVibration = min(leftVibration, 1.0f);
			rightVibration = min(rightVibration, 1.0f);

			leftVibrationOriginal = min(leftVibrationOriginal, 1.0f);
			rightVibrationOriginal = min(rightVibrationOriginal, 1.0f);

			pVibration->wLeftMotorSpeed = max(static_cast<std::uint16_t>(leftVibrationOriginal * 65535.0), static_cast<std::uint16_t>(leftVibration * 65535.0));
			pVibration->wRightMotorSpeed = max(static_cast<std::uint16_t>(rightVibrationOriginal * 65535.0), static_cast<std::uint16_t>(rightVibration * 65535.0));

			noise1timer += deltaTime * noise1speed;
		} else {
			long double leftVibrationOriginal = (double)pVibration->wLeftMotorSpeed / 65535;
			long double rightVibrationOriginal = (double)pVibration->wRightMotorSpeed / 65535;

			double smallAmp = (pow(1 + (smallRumble * smallAmpPreMult), smallAmpPow) - 1) * smallAmpPostMult;

			smallAmp = std::clamp(smallAmp, 0.0, 1.0);

			auto leftAdd = std::lerp(smallAmp * smallAmpLeftBalance, smallAmp, smallAmp);
			auto rightAdd = std::lerp(smallAmp, smallAmp * smallAmpRightBalance, smallAmp);

			leftVibrationOriginal += leftAdd + largeAmp;
			rightVibrationOriginal += rightAdd;

			leftVibrationOriginal = min(leftVibrationOriginal, 1.0f);
			rightVibrationOriginal = min(rightVibrationOriginal, 1.0f);

			pVibration->wLeftMotorSpeed = static_cast<std::uint16_t>(leftVibrationOriginal * 65535.0);
			pVibration->wRightMotorSpeed = static_cast<std::uint16_t>(rightVibrationOriginal * 65535.0);
		}
	}

	return false;
}
