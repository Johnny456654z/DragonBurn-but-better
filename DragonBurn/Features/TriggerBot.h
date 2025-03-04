#pragma once
#include <chrono>

#include "../Game/Game.h"
#include "../Game/Entity.h"
#include "../Core/Config.h"

namespace TriggerBot
{
	inline int TriggerDelay = 10; // ms
	inline int ShotDuration = 400; // ms
	inline bool ScopeOnly = true;
	inline bool IgnoreFlash = false;
	inline int HotKey = VK_XBUTTON2;
	inline std::chrono::time_point<std::chrono::system_clock> timepoint = std::chrono::system_clock::now();
	inline std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
	inline bool recorded = false;
	inline bool VisibleCheck = true;

	void Run(const CEntity& LocalEntity);
	bool CheckScopeWeapon(const std::string& WeaponName);
	std::string GetWeapon(const CEntity& LocalEntity);
	bool CheckWeapon(const std::string& WeaponName);
}