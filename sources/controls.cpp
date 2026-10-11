#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#include "F1Spirit.h"
#include "List.h"
#include "CPlayer.h"
#include "keyboardstate.h"
#include "filehandling.h"
#include "controls.h"

static int keys[CONTROLS_PLAYERS][CONTROLS_KEYS] = {
	{SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_SPACE, SDL_SCANCODE_M},
	{SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_LCTRL},
	{SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN},
	{SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN},
};
static int devices[CONTROLS_PLAYERS] = { -1, -1, -1, -1};


int Controls_get_key(int player, int key)
{
	if (player < 0 || player >= CONTROLS_PLAYERS || key < 0 || key >= CONTROLS_KEYS)
		return SDL_SCANCODE_UNKNOWN;

	return keys[player][key];
}


void Controls_set_key(int player, int key, int scancode)
{
	if (player >= 0 && player < CONTROLS_PLAYERS && key >= 0 && key < CONTROLS_KEYS)
		keys[player][key] = scancode;
}


int Controls_get_device(int player)
{
	if (player < 0 || player >= CONTROLS_PLAYERS)
		return -1;

	return devices[player];
}


void Controls_set_device(int player, int device)
{
	if (player >= 0 && player < CONTROLS_PLAYERS)
		devices[player] = device;
}


/* controls.cfg: one line per player, "<device> <6 scancodes>". */
bool Controls_load(void)
{
	FILE *fp = f1open("controls.cfg", "r", USERDATA);
	int i, j, v[CONTROLS_KEYS + 1];

	if (fp == 0)
		return false;

	for (i = 0;i < CONTROLS_PLAYERS;i++) {
		for (j = 0;j < CONTROLS_KEYS + 1;j++) {
			if (1 != fscanf(fp, "%i", &v[j])) {
				fclose(fp);
				return false;
			}
		}

		devices[i] = v[0];

		for (j = 0;j < CONTROLS_KEYS;j++)
			keys[i][j] = v[j + 1];
	}

	fclose(fp);
	return true;
}


bool Controls_save(void)
{
	FILE *fp = f1open("controls.cfg", "w", USERDATA);
	int i, j;

	if (fp == 0)
		return false;

	for (i = 0;i < CONTROLS_PLAYERS;i++) {
		fprintf(fp, "%i", devices[i]);

		for (j = 0;j < CONTROLS_KEYS;j++)
			fprintf(fp, " %i", keys[i][j]);

		fprintf(fp, "\n");
	}

	fclose(fp);
	return true;
}


void Controls_apply(CPlayerInfo *p)
{
	int i, j;

	for (i = 0;i < p->get_nplayers() && i < CONTROLS_PLAYERS;i++) {
		/* A gamepad that isn't plugged in would leave the car uncontrollable;
		   fall back to the keyboard keys for this race. */
		p->set_joystick(i, KEYBOARDSTATE::gamepad_connected(devices[i]) ? devices[i] : -1);

		for (j = 0;j < CONTROLS_KEYS;j++)
			p->set_key(i, j, keys[i][j]);
	}
}
