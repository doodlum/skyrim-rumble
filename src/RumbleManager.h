#pragma once

#include <shared_mutex>
#include <xinput.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <PerlinNoise.hpp>

class AnimationEventHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);
	static bool                      Register();
};

class FootstepEventHandler : public RE::BSTEventSink<RE::BGSFootstepEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::BGSFootstepEvent* a_event, RE::BSTEventSource<RE::BGSFootstepEvent>* a_eventSource);
	static bool                      Register();
};

class MenuOpenCloseEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource);
	static bool                      Register();
};

class RumbleManager
{
public:
	static void InstallHooks()
	{
		Hooks::Install();
	}

	static inline double SmoothBumpStep(double edge0, double edge1, double x)
	{
		x = 1.0 - abs(std::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0) - 0.5) * 2.0;
		return x * x * (3.0 - x - x);
	}

	static RumbleManager* GetSingleton()
	{
		static RumbleManager handler;
		return &handler;
	}

	struct Vibration
	{
		float        duration;
		RE::NiPoint2 motors;
	};

	enum VibrationType
	{
		kSmooth,
		kDiscrete,
		kBump
	};

	struct VibrationCustom
	{
		float         duration;
		RE::NiPoint2  motors;
		VibrationType type;
		float         time;
	};

	enum VibrationsCustom
	{
		FootstepFront,
		FootstepBack,
		FootstepWadingLeft,
		FootstepWadingRight,
		FootstepSprintLeft,
		FootstepSprintRight,
		VibrationsCustomCount
	};

	void FootstepEvent(const RE::BGSFootstepEvent* a_event);
	void AnimationEvent(const RE::BSAnimationGraphEvent* a_event);

	static float GetSubmergedLevelImpl(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell)
	{
		using func_t = decltype(&GetSubmergedLevelImpl);
		REL::Relocation<func_t> func{ REL::RelocationID(36452, 37448) };  // 1.5.97 1405E1510
		return func(a_actor, a_zPos, a_cell);
	}

	static float GetSubmergedLevel(RE::Actor* a_actor)
	{
		return GetSubmergedLevelImpl(a_actor, a_actor->GetPositionZ(), a_actor->GetParentCell());
	}

	static bool IsPowerAttacking(RE::Actor* a_actor)
	{
		using func_t = decltype(&IsPowerAttacking);
		REL::Relocation<func_t> func{ REL::RelocationID(37639, 38592) };  // 1.5.97 140627500
		return func(a_actor);
	}

	VibrationCustom Create(float duration, float left, float right, VibrationType type = VibrationType::kSmooth)
	{
		return VibrationCustom{ duration, RE::NiPoint2{ left, right }, type, 0 };
	}

	static void AddDiscreteRumbleImpl(std::int32_t type, float power, float duration)
	{
		using func_t = decltype(&AddDiscreteRumbleImpl);
		REL::Relocation<func_t> func{ REL::RelocationID(67220, 68528) };
		func(type, power, duration);
	}

	static void AddDiscreteRumble(RE::NiPoint2 power, float duration)
	{
		AddDiscreteRumbleImpl(0, power.x, duration);
		AddDiscreteRumbleImpl(1, power.y, duration);
	}

	std::shared_mutex mutex;
	
	bool enableMod = true;

	bool liveUpdate = false;

	float  smallRumble = 0;
	float  largeRumble = 0;
	bool   dataLoaded = false;
	bool   inLoadingScreen = false;
	bool   onGrindstone = false;
	double noise1timer = 0.0;

	siv::PerlinNoise noise1{ 1 };
	float            noiseSpeed = 100000.0f;

	float quadSprintMult = 1.5f;

	float powerAttackPow = 1.25f;
	float swingWeaponMult = 2.0f;

	bool  crossbowFix = true;
	float crossbowLeftMotor = 0.5f;
	float crossbowRightMotor = 1.0f;
	float crossbowDuration = 0.22f;

	float grindstonePow = 1.5f;
	float grindstoneLeftPower = 0.017f;
	float grindstoneRightPower= 0.017f;

	float submergedPow = 1.5f;
	float submergedLeftPower = 0.020f;
	float submergedRightPower= 0.020f;
	float submergedMountMult = 1.5f;

	float rainPow = 1.5f;
	float rainLeftPower = 0.015f;
	float rainRightPower= 0.020f;

	float smallAmpPreMult = 1.0f;
	float smallAmpPow = 3.0f;
	float smallAmpPostMult = 2.0f;
	float smallAmpLeftBalance = 1.0f;
	float smallAmpRightBalance = 0.0f;

	std::list<VibrationCustom>             activeVibrations;
	std::map<std::string, VibrationCustom> eventVibrations;

	int                        baseRumbleOverride = 1;
	std::map<std::string, int> vanillaOriginal;
	std::set<void*>            overrideForms;
	std::set<void*>            overrideCrossbows;
	std::string                animationEventName;
	std::string                overrideIdentifier;

	float largeAmp;

	void DataLoaded();

	struct VanillaOverride
	{
		std::string tag;
		int         rumbleSendValue;
	};
	std::map<std::string, VanillaOverride> vanillaOverrides;

	VibrationCustom sourcesCustom[VibrationsCustom::VibrationsCustomCount];

	void Trigger(VibrationsCustom type, float delay = 0)
	{
		std::lock_guard<std::shared_mutex> lk(mutex); 
		VibrationCustom source = sourcesCustom[type];
		source.time = source.duration + delay;
		activeVibrations.emplace_back(source);
	}
	void Trigger(VibrationCustom source, float delay = 0)
	{
		std::lock_guard<std::shared_mutex> lk(mutex); 
		source.time = source.duration + delay;
		activeVibrations.emplace_back(source);
	}

	void SetState(XINPUT_VIBRATION* pVibration);

	void AddFoley();
	void AddOverride();
	void Menu();

	void Load();
	void Save();

	void VanillaOverrideBase();

	void UpdateOverrides();

	bool ProcessHit(int type, float power, float duration);
	void DisableFeatures();

protected:
	struct Hooks
	{
		struct TESObjectREFR_ProcessHitEvent_AddDiscreteRumble
		{
			static void thunk(std::int32_t type, float power, float duration)
			{
				if (GetSingleton()->enableMod) {
					func(type, GetSingleton()->ProcessHit(type, power, duration) ? power : 0, duration);
				} else {
					func(type, power, duration);
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct BGSAudioMonitors_Update__powf_small
		{
			static float thunk(float x, float y)
			{
				GetSingleton()->smallRumble = x;
				return func(x, y);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct BGSAudioMonitors_Update__powf_large
		{
			static float thunk(float x, float y)
			{
				GetSingleton()->largeRumble = x;
				return func(x, y);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct BGSAudioMonitors_Update__Rumble_ModifyContinuous_SmallRumbleID
		{
			static void thunk(std::int32_t id, [[maybe_unused]] float power)
			{
				if (GetSingleton()->enableMod) {
					return func(id, 0);
				}
				return func(id, power);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct BGSAudioMonitors_Update__Rumble_ModifyContinuous_LargeRumbleID
		{
			static void thunk(std::int32_t id, [[maybe_unused]] float power)
			{
				GetSingleton()->largeAmp = power;
				if (GetSingleton()->enableMod) {
					return func(id, 0);
				}
				return func(id, power);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct XInput_SetState_Game
		{
			static DWORD thunk(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
			{
				if (GetSingleton()->enableMod) {
					GetSingleton()->SetState(pVibration);
				} else {
					GetSingleton()->DisableFeatures();
				}
				return func(dwUserIndex, pVibration);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<TESObjectREFR_ProcessHitEvent_AddDiscreteRumble>(REL::RelocationID(37633, 38586).address() + REL::Relocate(0xC80, 0xEC3));

			stl::write_thunk_call<BGSAudioMonitors_Update__powf_small>(REL::RelocationID(32314, 33058).address() + REL::Relocate(0x8F, 0x10C));
			stl::write_thunk_call<BGSAudioMonitors_Update__powf_large>(REL::RelocationID(32314, 33058).address() + REL::Relocate(0xB7, 0x134));

			stl::write_thunk_call<BGSAudioMonitors_Update__Rumble_ModifyContinuous_SmallRumbleID>(REL::RelocationID(32314, 33058).address() + REL::Relocate(0x121, 0x19E));
			stl::write_thunk_call<BGSAudioMonitors_Update__Rumble_ModifyContinuous_LargeRumbleID>(REL::RelocationID(32314, 33058).address() + REL::Relocate(0x12F, 0x1AC));

			stl::write_thunk_call<XInput_SetState_Game>(REL::RelocationID(67223, 68532).address() + REL::Relocate(0x25C, 0x313));
		}
	};

private:
	RumbleManager()
	{
		ZeroMemory(sourcesCustom, ARRAYSIZE(sourcesCustom));

		sourcesCustom[VibrationsCustom::FootstepFront] = Create(0.100f, 0.015f, 0.020f);
		sourcesCustom[VibrationsCustom::FootstepBack] = Create(0.100f, 0.020f, 0.015f);

		sourcesCustom[VibrationsCustom::FootstepWadingLeft] = Create(1.000f, 0.015f, 0.000f);
		sourcesCustom[VibrationsCustom::FootstepWadingRight] = Create(1.000f, 0.000f, 0.015f);

		sourcesCustom[VibrationsCustom::FootstepSprintLeft] = Create(0.100f, 0.015f, 0.010f);
		sourcesCustom[VibrationsCustom::FootstepSprintRight] = Create(0.100f, 0.010f, 0.015f);

		Load();
	}

	RumbleManager(const RumbleManager&) = delete;
	RumbleManager(RumbleManager&&) = delete;

	~RumbleManager() = default;

	RumbleManager& operator=(const RumbleManager&) = delete;
	RumbleManager& operator=(RumbleManager&&) = delete;
};
