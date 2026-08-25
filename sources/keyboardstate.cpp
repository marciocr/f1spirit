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


KEYBOARDSTATE::KEYBOARDSTATE(void)
{
	int i;
	SDL_JoystickID *ids;

	ids = SDL_GetGamepads(&n_joysticks);

	if (n_joysticks > 0) {
		joysticks = new SDL_Gamepad * [n_joysticks];

		for (i = 0;i < n_joysticks;i++)
			joysticks[i] = SDL_OpenGamepad(ids[i]);
	} else {
		joysticks = 0;
	}

	SDL_free(ids);

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
	int i;

	delete []keyboard;
	keyboard = 0;
	delete []old_keyboard;
	old_keyboard = 0;

	for (i = 0;i < n_joysticks;i++) {
		SDL_CloseGamepad(joysticks[i]);
		joysticks[i] = 0;
	}

	delete []joysticks;


}


void KEYBOARDSTATE::cycle(void)
{
	int i, j, v;
	const bool *k = SDL_GetKeyboardState(NULL);

	/* Update keyboard: */

	for (i = 0;i < SDL_SCANCODE_COUNT;i++) {
		old_keyboard[i] = keyboard[i];
		keyboard[i] = k[i] ? 1 : 0;
	}

	if (n_joysticks > 0) {
		SDL_UpdateGamepads();

		/* Update joysticks: */

		for (i = 0;i < n_joysticks;i++) {
			for (j = 0;j < joystick_size;j++)
				old_keyboard[joystick_0_pos + i*joystick_size + j] = keyboard[joystick_0_pos + i * joystick_size + j];

			v = SDL_GetGamepadAxis(joysticks[i], SDL_GAMEPAD_AXIS_LEFTX);

			if (v < -16000)
				keyboard[joystick_0_pos + i*joystick_size] = 1;
			else
				keyboard[joystick_0_pos + i*joystick_size] = 0;

			if (v > 16000)
				keyboard[joystick_0_pos + i*joystick_size + 1] = 1;
			else
				keyboard[joystick_0_pos + i*joystick_size + 1] = 0;

			v = SDL_GetGamepadAxis(joysticks[i], SDL_GAMEPAD_AXIS_LEFTY);

			if (v < -16000)
				keyboard[joystick_0_pos + i*joystick_size + 2] = 1;
			else
				keyboard[joystick_0_pos + i*joystick_size + 2] = 0;

			if (v > 16000)
				keyboard[joystick_0_pos + i*joystick_size + 3] = 1;
			else
				keyboard[joystick_0_pos + i*joystick_size + 3] = 0;

			for (j = 0;j < 8;j++) {
				keyboard[joystick_0_pos + i*joystick_size + 4 + j] = SDL_GetGamepadButton(joysticks[i], joystick_gamepad_buttons[j]) ? 1 : 0;
			}
		}
	}

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

	if (4 != fscanf(fp, "%i %i %i %i", &n_joysticks, &k_size, &joystick_0_pos, &joystick_size))
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

			keyboard[i] = j;

			old_keyboard[i] = k;
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
