#pragma once

#include <PerlinNoise.hpp>
#include <xinput.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

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

class HitEventHandler : public RE::BSTEventSink<RE::TESHitEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource);
	static bool Register();
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

	bool   liveUpdate = false;

	float  smallRumble = 0;
	float  largeRumble = 0;
	bool   dataLoaded = false;
	bool   onGrindstone = false;
	double noise1timer = 0.0;
	float  hitTime = 0.0f;
	float  hitBlockedTime = 0.0f;

	siv::PerlinNoise noise1{ 1 };
	float            noise1speed = 100000.0f;

	float quadSprintMult = 1.25f;

	float grindstoneSmallPower = 0.020f;
	float grindstonePow = 1.5f;
	float grindstoneLargePower = 0.020f;

	float submergedSmallPower = 0.020f;
	float submergedPow = 1.5f;
	float submergedLargePower = 0.020f;
	float submergedMountMult = 1.5f;

	float rainSmallPower = 0.015f;
	float rainPow = 1.5f;
	float rainLargePower = 0.020f;

	float smallAmpPreMult = 1.0f;
	float smallAmpPow = 4.0f;
	float smallAmpPostMult = 1.0f;
	float smallAmpLeftBalance = 1.0f;
	float smallAmpRightBalance = 0.0f;

	float powerAttackPow = 1.25f;

	float hitPow = 20.0f;
	float hitDuration = 0.25f;

	float hitBlockedPow = 10.0f;
	float hitBlockedDuration = 0.25f;

	float swingWeaponMult = 2.0f;
	float castingMult = 0.5f;

	std::list<VibrationCustom>             activeVibrations;
	std::map<std::string, VibrationCustom> eventVibrations;

	int                        baseRumbleOverride = 1;
	std::map<std::string, int> vanillaOriginal;
	std::set<void*>            overrideForms;
	std::string                animationEventName;
	std::string                overrideIdentifier;

	void DataLoaded();
	;

	struct VanillaOverride
	{
		std::string tag;
		int         rumbleSendValue;
	};
	std::map<std::string, VanillaOverride> vanillaOverrides;

	VibrationCustom sourcesCustom[VibrationsCustom::VibrationsCustomCount];

	void Trigger(VibrationsCustom type, float delay = 0)
	{
		VibrationCustom source = sourcesCustom[type];
		source.time = source.duration + delay;
		activeVibrations.emplace_back(source);
	}
	void Trigger(VibrationCustom source, float delay = 0)
	{
		source.time = source.duration + delay;
		activeVibrations.emplace_back(source);
	}

	bool SetState(XINPUT_VIBRATION* pVibration);

	void AddFoley();
	void AddOverride();
	void Menu();

	void LoadJSON();
	void SaveJSON();

	void VanillaOverrideBase();

	void UpdateOverrides();

	bool ProcessHit(int type, float power, float duration);

protected:
	struct Hooks
	{
		struct PCProcessAnimGraphEvent_ProcessEvent
		{
			static RE::BSEventNotifyControl thunk(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_this, RE::BSAnimationGraphEvent& a_event,
				RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_src)
			{
				GetSingleton()->AnimationEvent(&a_event);
				return func(a_this, a_event, a_src);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct TESObjectREFR_ProcessHitEvent_AddDiscreteRumble
		{
			static void thunk(std::int32_t type, float power, float duration)
			{
				func(type, GetSingleton()->ProcessHit(type, power, duration) ? power : 0, duration);
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
				return func(id, 0);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct BGSAudioMonitors_Update__Rumble_ModifyContinuous_LargeRumbleID
		{
			static void thunk(std::int32_t id, [[maybe_unused]] float power)
			{
				return func(id, power);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct XInput_SetState_Game
		{
			static DWORD thunk(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
			{
				GetSingleton()->SetState(pVibration);
				return func(dwUserIndex, pVibration);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			REL::Relocation<uintptr_t> PCProcessAnimGraphEventVtbl{ RE::VTABLE_PlayerCharacter[2] };
			PCProcessAnimGraphEvent_ProcessEvent::func = PCProcessAnimGraphEventVtbl.write_vfunc(0x1, &PCProcessAnimGraphEvent_ProcessEvent::thunk);

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

		sourcesCustom[VibrationsCustom::FootstepFront] = Create(0.050f, 0.050f, 0.025f);
		sourcesCustom[VibrationsCustom::FootstepBack] = Create(0.100f, 0.025f, 0.050f);

		sourcesCustom[VibrationsCustom::FootstepWadingLeft] = Create(1.000f, 0.011f, 0.000f);
		sourcesCustom[VibrationsCustom::FootstepWadingRight] = Create(1.000f, 0.000f, 0.011f);

		LoadJSON();
	}

	RumbleManager(const RumbleManager&) = delete;
	RumbleManager(RumbleManager&&) = delete;

	~RumbleManager() = default;

	RumbleManager& operator=(const RumbleManager&) = delete;
	RumbleManager& operator=(RumbleManager&&) = delete;
};
