#include "..\hooks.hpp"
#include "..\..\cheats\visuals\dormant_esp.h"

using onsoundstart_t = void(__thiscall*)(void*, int32_t, StartSoundParams_t*, const char*);

void __stdcall hooks::hooked_onsoundstart(void* pEcx, void* pEdx, int32_t iGuid, StartSoundParams_t* pParams, const char* kchpSoundname)
{
    static auto original_fn = soundservice_hook->get_func_address <onsoundstart_t>(27);
    original_fn(pEcx, iGuid, pParams, kchpSoundname);

    g_ctx.globals.dormant_by_hook = true;

    if (pParams->soundsource < 0 || pParams->soundsource > 64 || !pParams->origin.IsValid())
        return;

    auto e = (player_t*)m_entitylist()->GetClientEntity(pParams->soundsource);

    if (!e->valid(true, false))
        return;

    if (!e->IsDormant())
        return;

    c_dormant_esp::get().m_cSoundPlayers[pParams->soundsource].m_iReceiveTime = m_globals()->m_realtime;
    c_dormant_esp::get().m_cSoundPlayers[pParams->soundsource].m_nFlags = pParams->flags;
    c_dormant_esp::get().m_cSoundPlayers[pParams->soundsource].m_vecOrigin = pParams->origin;
}