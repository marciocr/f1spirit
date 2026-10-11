#ifndef __F1SPIRIT_CONTROLS_H
#define __F1SPIRIT_CONTROLS_H

/* Input configuration for the (up to) 4 local players, set from OPTIONS ->
   CONTROLS and saved in controls.cfg. It belongs to the machine, not to a
   player profile: Controls_apply() copies it into the profile when a race
   starts, so replays keep the bindings they were recorded with. */
#define CONTROLS_PLAYERS 4
#define CONTROLS_KEYS 6	/* gear up, gear down, left, right, accelerate, brake */

int Controls_get_key(int player, int key);
void Controls_set_key(int player, int key, int scancode);
int Controls_get_device(int player);	/* -1 = keyboard, else gamepad slot */
void Controls_set_device(int player, int device);

bool Controls_load(void);
bool Controls_save(void);
void Controls_apply(class CPlayerInfo *p);

#endif
