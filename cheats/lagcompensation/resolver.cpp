// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\ragebot\aim.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch)
{
	player = e;
	player_record = record;

	original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
	original_pitch = math::normalize_pitch(pitch);
}

void resolver::reset()
{
	player = nullptr;
	player_record = nullptr;

	side = false;
	fake = false;

	was_first_bruteforce = false;
	was_second_bruteforce = false;

	original_goal_feet_yaw = 0.0f;
	original_pitch = 0.0f;
}

void resolver::resolve_yaw()
{
	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(player->EntIndex(), &player_info))
		return;

#if RELEASE
	if (player_info.fakeplayer || !g_ctx.local()->is_alive() || player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
#else
	if (!g_ctx.local()->is_alive() || player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
#endif
	{ // do we need to resolve player?
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	// shit player?
	auto animstate = player->get_animation_state();
	if (!animstate)
	{
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	// some bools to history checks
	bool is_player_zero = false;
	bool is_player_faking = false;
	int positives = 0;
	int negatives = 0;

	fake = false;

	// save history res type
	resolver_history res_history = HISTORY_UNKNOWN;

	// do we have record for that player in history?
	for (auto it = lagcompensation::get().player_sets.begin(); it != lagcompensation::get().player_sets.end(); ++it)
		if (it->id == player_info.steamID64)
		{
			res_history = it->res_type;
			is_player_faking = it->faking;
			positives = it->pos;
			negatives = it->neg;

			break;
		}

	// did we hit player in low or zero side?

	if (res_history == HISTORY_ZERO) 
		is_player_zero = true;

	// logical side detect
	resolver_type type = resolver_type(-1);

	auto valid_move = true;
	if (animstate->m_velocity > 0.1f)
		valid_move = animstate->m_flTimeSinceStartedMoving < 0.22f;
	
	// standing?
	if (valid_move && player_record->layers[3].m_flWeight == 0.f && player_record->layers[3].m_flCycle == 0.f &&
		player_record->layers[6].m_flWeight == 0.f)
	{
		auto m_delta = math::angle_diff(player->m_angEyeAngles().y, gfy_default);
		side = 2 * (m_delta <= 0.0f) - 1;
		type = resolver_type::ANIM_s;
	}
	else if (!valid_move &&
			!(int(player_record->layers[12].m_flWeight * 1000.f)) &&
			int(player_record->layers[6].m_flWeight * 1000.f) == int(previous_layers[6].m_flWeight * 1000.f))
	{

		float delta_first = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[0][6].m_flPlaybackRate);
		float delta_second = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[2][6].m_flPlaybackRate);
		float delta_third = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[1][6].m_flPlaybackRate);

		if (delta_first < delta_second || delta_third <= delta_second || (delta_second * 1000.f))
		{
			if (delta_first >= delta_third && delta_second > delta_third && !(delta_third * 1000.f))
				side = 1;
		}
		else
			side = -1;

		type = resolver_type::ANIM_m;


		/*
		// 0 - fake
		// 1 - negative
		// 2 - positive
		v16 = int(record->layers[6].playbackRate - delta_layers[0][6].playbackRate);
		v17 = int(record->layers[6].playbackRate - delta_layers[2][6].playbackRate);
		v18 = int(record->layers[6].playbackRate - delta_layers[1][6].playbackRate);
		if (v16 < v17 || v18 <= v17 || (v17 * 1000.f))
		{
			if (v16 >= v18 && v17 > v18 && !(v18 * 1000.f))
			{
				record->unk_6 = 1;
				record->side = 1;
			}
		}
		else
		{
			record->unk_6 = 1;
			record->side = -1;
		}
		*/

	}
	else
	{
		if (m_globals()->m_curtime - lock_side > 2.0f)
		{
			// lets compare two sides
			auto fire_data_first = autowall::get().wall_penetration(g_ctx.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.first), player);
			auto fire_data_second = autowall::get().wall_penetration(g_ctx.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.second), player);

			// fake head check
			if (fire_data_first.visible != fire_data_second.visible)
			{
				type = resolver_type::TRACE;
				side = fire_data_second.visible;
				lock_side = m_globals()->m_curtime;
			}
			else
			{
				type = resolver_type::DIRECTIONAL;

				// history check
				if (abs(positives - negatives) > 3)
					side = positives > negatives;

				// comparnig with damage 
				else if (fire_data_first.damage != fire_data_second.damage)
					side = fire_data_first.damage < fire_data_second.damage;
			}
		}
		else
			type = resolver_type::LOCKED_SIDE;

	}

	// some code for semi-proper side-switch
	if (g_ctx.globals.missed_shots[player->EntIndex()])
	{
		switch (last_side)
		{
		case RESOLVER_ORIGINAL:
			g_ctx.globals.missed_shots[player->EntIndex()] = 0;
			fake = true;
			break;
		case RESOLVER_ZERO:
			player_record->type = BRUTEFORCE;

			if (!is_player_zero)
				player_record->side = side ? RESOLVER_LOW_FIRST : RESOLVER_LOW_SECOND;
			else
				player_record->side = side ? RESOLVER_FIRST : RESOLVER_SECOND;

			was_first_bruteforce = false;
			was_second_bruteforce = false;
			return;
		case RESOLVER_FIRST:
			player_record->type = BRUTEFORCE;
			player_record->side = was_second_bruteforce ? RESOLVER_ZERO : RESOLVER_SECOND;

			was_first_bruteforce = true;

			was_first_low_bruteforce = false;
			was_second_low_bruteforce = false;
			return;
		case RESOLVER_SECOND:
			player_record->type = BRUTEFORCE;
			player_record->side = was_first_bruteforce ? RESOLVER_ZERO : RESOLVER_FIRST;

			was_second_bruteforce = true;

			was_first_low_bruteforce = false;
			was_second_low_bruteforce = false;
			return;
		case RESOLVER_LOW_FIRST:
			player_record->type = BRUTEFORCE;
			player_record->side = was_second_low_bruteforce ? (side ? RESOLVER_FIRST : RESOLVER_SECOND) : RESOLVER_LOW_SECOND;

			was_first_low_bruteforce = true;
			return;
		case RESOLVER_LOW_SECOND:
			player_record->type = BRUTEFORCE;
			player_record->side = was_first_low_bruteforce ? (side ? RESOLVER_FIRST : RESOLVER_SECOND) : RESOLVER_LOW_FIRST;

			was_second_low_bruteforce = true;
			return;
		}
	}


	// get player's choke amount
	auto choked = abs(TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime()) - 1);

	// is he possibly using desync?
	if (fabs(original_pitch) > 65.f || choked >= 1 || is_player_faking)
		fake = true;
	else if (!fake && !g_ctx.globals.missed_shots[player->EntIndex()])
	{
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	// set new resolver type & side
	if (side)
	{
		player_record->type = type;
		player_record->side = RESOLVER_FIRST;
	}
	else
	{
		player_record->type = type;
		player_record->side = RESOLVER_SECOND;
	}

	// does we have record for that player in history?
	if (is_player_zero && !g_ctx.globals.missed_shots[player->EntIndex()])
		player_record->side = RESOLVER_ZERO;

}

float resolver::resolve_pitch()
{
	return original_pitch;
}