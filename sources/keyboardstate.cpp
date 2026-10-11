#include <assert.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "List.h"

#include "keyboardstate.h"


#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif

#define JOYSTICK_SIZE 12

/* Fixed order for the 8 button slots each joystick gets in the keyboard array
   (slots joystick_0_pos + i*JOYSTICK_SIZE + 4 .. +11). Using SDL_Gamepad's
   standardized button IDs instead of raw per-device button indices means
   "button 0"/"button 1" (accelerate/brake, see state_gameoptions.cpp) always
   land on the same physical buttons (e.g. Xbox A/B, PlayStation Cross/Circle)
   regardless of controller make. */
static const SDL_GamepadButton joystick_gamepad_buttons[8] = {
	SDL_GAMEPAD_BUTTON_SOUTH,
	SDL_GAMEPAD_BUTTON_EAST,
	SDL_GAMEPAD_BUTTON_WEST,
	SDL_GAMEPAD_BUTTON_NORTH,
	SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
	SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
	SDL_GAMEPAD_BUTTON_BACK,
	SDL_GAMEPAD_BUTTON_START,
};


static SDL_Gamepad *gamepad_slots[MAX_GAMEPADS] = {0};

bool KEYBOARDSTATE::gamepad_menu_navigation = false;

/* Buttons held when menu navigation switches on (e.g. still accelerating as
   the race ends) don't count as menu presses until they're released. */
static bool menu_navigation_was_on = false;
static unsigned char menu_navigation_latched[MAX_GAMEPADS][JOYSTICK_SIZE];


void KEYBOARDSTATE::gamepad_added(SDL_JoystickID id)
{
	int i;

	for (i = 0;i < MAX_GAMEPADS;i++) {
		if (gamepad_slots[i] != 0 && SDL_GetGamepadID(gamepad_slots[i]) == id)
			return;
	}

	for (i = 0;i < MAX_GAMEPADS;i++) {
		if (gamepad_slots[i] == 0) {
			gamepad_slots[i] = SDL_OpenGamepad(id);
			return;
		}
	}
}


void KEYBOARDSTATE::gamepad_removed(SDL_JoystickID id)
{
	int i;

	for (i = 0;i < MAX_GAMEPADS;i++) {
		if (gamepad_slots[i] != 0 && SDL_GetGamepadID(gamepad_slots[i]) == id) {
			SDL_CloseGamepad(gamepad_slots[i]);
			gamepad_slots[i] = 0;
		}
	}
}


bool KEYBOARDSTATE::gamepad_connected(int slot)
{
	return slot >= 0 && slot < MAX_GAMEPADS && gamepad_slots[slot] != 0;
}


void KEYBOARDSTATE::close_gamepads(void)
{
	int i;

	for (i = 0;i < MAX_GAMEPADS;i++) {
		if (gamepad_slots[i] != 0)
			SDL_CloseGamepad(gamepad_slots[i]);

		gamepad_slots[i] = 0;
	}
}


void KEYBOARDSTATE::device_name(int slot, char *buf)
{
	if (slot < 0)
		strcpy(buf, "KEYBOARD");
	else if (gamepad_connected(slot))
		sprintf(buf, "GAMEPAD %i", slot + 1);
	else
		sprintf(buf, "GAMEPAD %i OFF", slot + 1);
}


void KEYBOARDSTATE::create_device_menu(char **options, int *noptions)
{
	char tmp[32];
	int i;

	*options = new char[(MAX_GAMEPADS + 2) * 32];
	strcpy(*options, "KEYBOARD\n");

	for (i = 0;i < MAX_GAMEPADS;i++) {
		device_name(i, tmp);
		strcat(*options, tmp);
		strcat(*options, "\n");
	}

	strcat(*options, "BACK\n");
	*noptions = MAX_GAMEPADS + 2;
}


KEYBOARDSTATE::KEYBOARDSTATE(void)
{
	int i;

	n_joysticks = MAX_GAMEPADS;

	k_size = SDL_SCANCODE_COUNT + n_joysticks * JOYSTICK_SIZE;

	joystick_0_pos = SDL_SCANCODE_COUNT;

	joystick_size = JOYSTICK_SIZE;

	keyboard = new unsigned char[k_size];

	old_keyboard = new unsigned char[k_size];

	for (i = 0;i < k_size;i++) {
		keyboard[i] = 0;
		old_keyboard[i] = 0;
	}
}


KEYBOARDSTATE::~KEYBOARDSTATE(void)
{
	delete []keyboard;
	keyboard = 0;
	delete []old_keyboard;
	old_keyboard = 0;
}


void KEYBOARDSTATE::cycle(void)
{
	int i, j, v;
	const bool *k = SDL_GetKeyboardState(NULL);
	SDL_Gamepad *g;

	/* Update keyboard: */

	for (i = 0;i < SDL_SCANCODE_COUNT;i++) {
		old_keyboard[i] = keyboard[i];
		keyboard[i] = k[i] ? 1 : 0;
	}

	SDL_UpdateGamepads();

	/* Update joysticks (the d-pad counts as the stick): */

	for (i = 0;i < n_joysticks;i++) {
		unsigned char *slot = keyboard + joystick_0_pos + i * joystick_size;

		for (j = 0;j < joystick_size;j++) {
			old_keyboard[joystick_0_pos + i*joystick_size + j] = slot[j];
			slot[j] = 0;
		}

		g = gamepad_slots[i];

		if (g == 0)
			continue;

		v = SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTX);

		slot[0] = (v < -16000 || SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) ? 1 : 0;
		slot[1] = (v > 16000 || SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) ? 1 : 0;

		v = SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTY);

		slot[2] = (v < -16000 || SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_UP)) ? 1 : 0;
		slot[3] = (v > 16000 || SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) ? 1 : 0;

		for (j = 0;j < 8;j++) {
			slot[4 + j] = SDL_GetGamepadButton(g, joystick_gamepad_buttons[j]) ? 1 : 0;
		}

		/* Mirror into the keyboard keys the menus/screens listen to: */
		if (gamepad_menu_navigation) {
			unsigned char *latched = menu_navigation_latched[i];
			unsigned char b[JOYSTICK_SIZE];

			for (j = 0;j < joystick_size;j++) {
				if (!menu_navigation_was_on)
					latched[j] = slot[j];

				latched[j] &= slot[j];
				b[j] = slot[j] & !latched[j];
			}

			keyboard[SDL_SCANCODE_LEFT] |= b[0];
			keyboard[SDL_SCANCODE_RIGHT] |= b[1];
			keyboard[SDL_SCANCODE_UP] |= b[2];
			keyboard[SDL_SCANCODE_DOWN] |= b[3];
			keyboard[SDL_SCANCODE_SPACE] |= b[4] | b[11];	/* A, Start */
			keyboard[SDL_SCANCODE_ESCAPE] |= b[5] | b[10];	/* B, Back */
		} else {
			keyboard[SDL_SCANCODE_ESCAPE] |= slot[11];	/* Start */
		}
	}

	menu_navigation_was_on = gamepad_menu_navigation;
}


void KEYBOARDSTATE::copy(KEYBOARDSTATE *lvk)
{
	int i;
	List<int> l;
	int *ks, *ks2;

	LVK_ID = lvk->LVK_ID;

	for (i = 0;i < k_size && i < lvk->k_size;i++) {
		keyboard[i] = lvk->keyboard[i];
		old_keyboard[i] = lvk->old_keyboard[i];
	}

	l.Instance(lvk->keyevents);

	l.Rewind();

	while (l.Iterate(ks)) {
		ks2 = new int(*ks);
		keyevents.Add(ks2);
	}

}


bool KEYBOARDSTATE::save(FILE *fp)
{
	int i;
	List<int> l;
	int *ks;

	fprintf(fp, "%i %i %i %i\n", n_joysticks, k_size, joystick_0_pos, joystick_size);

	for (i = 0;i < k_size;i++) {
		if (keyboard[i] ||
		        old_keyboard[i]) {
			fprintf(fp, "K %i %i %i\n", i, int(keyboard[i]), int(old_keyboard[i]));
		}
	}

	fprintf(fp, "%i\n", keyevents.Length());

	l.Instance(keyevents);

	l.Rewind();

	while (l.Iterate(ks)) {
		fprintf(fp, "%i\n", *ks);
	}

	return true;
}


bool KEYBOARDSTATE::load(FILE *fp)
{
	int i, j, k, n;
	int *ks;
	char tmp[32];

	int file_n_joysticks, file_k_size, file_joystick_0_pos, file_joystick_size;

	/* The layout is fixed now (MAX_GAMEPADS slots); older replays recorded the
	   number of gamepads connected at the time, so only keep inputs that fit. */
	if (4 != fscanf(fp, "%i %i %i %i", &file_n_joysticks, &file_k_size, &file_joystick_0_pos, &file_joystick_size))
		return false;

	for (i = 0;i < k_size;i++) {
		keyboard[i] = 0;
		old_keyboard[i] = 0;
	}

	keyevents.Delete();

	do {
		if (1 != fscanf(fp, "%s", tmp))
			return false;

		if (strcmp(tmp, "K") == 0) {
			if (1 != fscanf(fp, "%i", &i))
				return false;

			if (2 != fscanf(fp, "%i %i", &j, &k))
				return false;

			if (i >= 0 && i < k_size) {
				keyboard[i] = j;
				old_keyboard[i] = k;
			}
		} else {
			if (1 != sscanf(tmp, "%i", &i))
				return false;

			for (j = 0;j < i;j++) {
				if (1 != fscanf(fp, "%i", &n))
					return false;

				ks = new int(n);

				keyevents.Add(ks);
			}
		}

	} while (strcmp(tmp, "K") == 0);

	return true;
}
