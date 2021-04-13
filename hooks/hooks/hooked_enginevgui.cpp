#include "..\hooks.hpp"
#include "..\..\cheats\menu.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\visuals\player_esp.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\visuals\world_esp.h"
#include "..\..\cheats\misc\misc.h"
#include "..\..\cheats\misc\walkbot.h"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\visuals\bullet_tracers.h"
#include "..\..\cheats\visuals\dormant_esp.h"
#include "..\..\cheats\lagcompensation\local_animations.h"

typedef void(__thiscall* Paint_t)(IEngineVGui*, int);
typedef void(__thiscall* start_drawing)(void*);
typedef void(__thiscall* finish_drawing)(void*);

void __stdcall hooks::hooked_enginevgui(int mode)
{
	static auto original_fn = enginevgui_hook->get_func_address <Paint_t>(14);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	static start_drawing start_draw = (start_drawing)util::FindSignature("vguimatsurface.dll", "55 8B EC 83 E4 C0 83 EC 38");
	static finish_drawing end_draw = (finish_drawing)util::FindSignature("vguimatsurface.dll", "8B 0D ? ? ? ? 56 C6 05");

	if (mode & 1)
	{
		start_draw(m_surface());

		end_draw(m_surface());
	}
}