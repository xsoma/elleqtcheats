// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "key_binds.h"
#include "..\..\includes.hpp"
#include "misc.h"

void key_binds::update_key_bind(key_bind* key_bind, int key_bind_id)
{
	auto is_button_down = util::is_button_down(key_bind->key);

	switch (key_bind->mode)
	{
	case HOLD:
		switch (key_bind_id)
		{
		case KB_DOUBLETAP:
			if (misc::get().recharging_double_tap)
				break;

			misc::get().double_tap_key = is_button_down;

			if (misc::get().double_tap_key && g_cfg.ragebot.double_tap_key.key != g_cfg.antiaim.hide_shots_key.key)
				misc::get().hide_shots_key = false;

			break;
		case KB_HIDESHOTS:
			misc::get().hide_shots_key = is_button_down;

			if (misc::get().hide_shots_key && g_cfg.antiaim.hide_shots_key.key != g_cfg.ragebot.double_tap_key.key)
				misc::get().double_tap_key = false;

			break;
		case KB_MANUALBACK:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_BACK;
			else if (antiaim::get().manual_side == SIDE_BACK)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		case KB_MANUALLEFT:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_LEFT;
			else if (antiaim::get().manual_side == SIDE_LEFT)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		case KB_MANUALRIGHT:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_RIGHT;
			else if (antiaim::get().manual_side == SIDE_RIGHT)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		default:
			keys[key_bind_id] = is_button_down;
			break;
		}
		
		break;
	case TOGGLE:
		if (!key_bind->holding && is_button_down)
		{
			switch (key_bind_id)
			{
			case KB_DOUBLETAP:
				if (misc::get().recharging_double_tap)
					break;

				misc::get().double_tap_key = !misc::get().double_tap_key;

				if (misc::get().double_tap_key && g_cfg.ragebot.double_tap_key.key != g_cfg.antiaim.hide_shots_key.key)
					misc::get().hide_shots_key = false;

				break;
			case KB_HIDESHOTS:
				misc::get().hide_shots_key = !misc::get().hide_shots_key;

				if (misc::get().hide_shots_key && g_cfg.antiaim.hide_shots_key.key != g_cfg.ragebot.double_tap_key.key)
					misc::get().double_tap_key = false;

				break;
			case KB_MANUALBACK:
				if (antiaim::get().manual_side == SIDE_BACK)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_BACK;

				break;
			case KB_MANUALLEFT:
				if (antiaim::get().manual_side == SIDE_LEFT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_LEFT;

				break;
			case KB_MANUALRIGHT:
				if (antiaim::get().manual_side == SIDE_RIGHT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_RIGHT;

				break;
			default:
				keys[key_bind_id] = !keys[key_bind_id];
				break;
			}

			key_bind->holding = true;
		}
		else if (key_bind->holding && !is_button_down)
			key_bind->holding = false;

		break;
	}

	mode[key_bind_id] = key_bind->mode;
}

void key_binds::initialize_key_binds()
{
	for (auto i = 0; i < 24; i++)
	{
		keys[i] = false;

		if (i == KB_DOUBLETAP || i >= KB_HIDESHOTS && i <= KB_THIRDPERSON)
			mode[i] = TOGGLE;
		else
			mode[i] = HOLD;
	}
}

void key_binds::update_key_binds()
{
	update_key_bind(&g_cfg.legitbot.autofire_key, KB_AUTOFIRE);
	update_key_bind(&g_cfg.legitbot.key, KB_LEGITKEY);
	update_key_bind(&g_cfg.ragebot.double_tap_key, KB_DOUBLETAP);
	update_key_bind(&g_cfg.ragebot.safe_point_key, KB_SAFEPOINTS);

	for (auto i = 0; i < 8; i++)
		update_key_bind(&g_cfg.ragebot.weapon[i].damage_override_key, 4 + i);

	update_key_bind(&g_cfg.antiaim.hide_shots_key, KB_HIDESHOTS);
	update_key_bind(&g_cfg.antiaim.manual_back, KB_MANUALBACK);
	update_key_bind(&g_cfg.antiaim.manual_left, KB_MANUALLEFT);
	update_key_bind(&g_cfg.antiaim.manual_right, KB_MANUALRIGHT);
	update_key_bind(&g_cfg.antiaim.flip_desync, KB_FLIPDESYNC);
	update_key_bind(&g_cfg.misc.thirdperson_toggle, KB_THIRDPERSON);
	update_key_bind(&g_cfg.misc.automatic_peek, KB_AUTOPEEK);
	update_key_bind(&g_cfg.misc.edge_jump, KB_EDGEJUMP);
	update_key_bind(&g_cfg.misc.fakeduck_key, KB_FAKEDUCK);
	update_key_bind(&g_cfg.misc.slowwalk_key, KB_SLOWWALK);
	update_key_bind(&g_cfg.ragebot.body_aim_key, KB_BODYAIM);
	update_key_bind(&g_cfg.misc.walkbot_key, KB_WALKBOT);
}

void key_binds::set_key_bind_state(int key_bind_id, bool value)
{
	keys[key_bind_id] = value;
}

bool key_binds::get_key_bind_state(int key_bind_id)
{
	return keys[key_bind_id];
}

bool key_binds::get_key_bind_state_lua(int key_bind_id)
{
	if (key_bind_id < 0 || key_bind_id > 23)
		return false;

	switch (key_bind_id)
	{
	case KB_DOUBLETAP:
		return misc::get().double_tap_key;
	case 4: // Damage override key
		if (g_ctx.globals.current_weapon < 0)
			return false;

		return keys[4 + g_ctx.globals.current_weapon];
	case KB_HIDESHOTS:
		return misc::get().hide_shots_key;
	case KB_MANUALBACK:
		return antiaim::get().manual_side == SIDE_BACK;
	case KB_MANUALLEFT:
		return antiaim::get().manual_side == SIDE_LEFT;
	case KB_MANUALRIGHT:
		return antiaim::get().manual_side == SIDE_RIGHT;
	default:
		return keys[key_bind_id];
	}
}

bool key_binds::get_key_bind_mode(int key_bind_id)
{
	return mode[key_bind_id];
}