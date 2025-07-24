#pragma once
#include "..\Game/Game.h"
#include "..\Game/Entity.h"
#include "..\Features\Aimbot.h"
#include "..\Features/Radar.h"
#include "..\Features/TriggerBot.h"
#include "..\Features/Misc.h"

namespace Cheats
{

	void Run();
}

struct {
	ImFont* normal15px = nullptr;
} fonts;