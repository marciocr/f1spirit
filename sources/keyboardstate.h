#ifndef _BRAIN_KEYBOARDSTATE
#define _BRAIN_KEYBOARDSTATE

/* Gamepads live in fixed slots so a player's binding ("GAMEPAD 2") survives
   other controllers being plugged or unplugged. */
#define MAX_GAMEPADS 4

class KEYBOARDSTATE
{

	public:
		KEYBOARDSTATE();
		~KEYBOARDSTATE();

		void copy(KEYBOARDSTATE *lvk);

		void cycle(void);

		bool save(FILE *fp);
		bool load(FILE *fp);

		/* Hotplug: called from the main loop on SDL_EVENT_GAMEPAD_ADDED/REMOVED
		   (SDL also sends ADDED for controllers already connected at startup). */
		static void gamepad_added(SDL_JoystickID id);
		static void gamepad_removed(SDL_JoystickID id);
		static bool gamepad_connected(int slot);
		static void close_gamepads(void);

		/* Names for a player's input device (-1 = keyboard, else gamepad slot),
		   and the KEYBOARD / GAMEPAD 1..MAX_GAMEPADS / BACK menu to pick one. */
		static void device_name(int slot, char *buf);
		static void create_device_menu(char **options, int *noptions);

		/* When set, any connected gamepad also drives the menu keys (d-pad/stick
		   -> arrows, A/Start -> space, B/Back -> escape); otherwise only Start is
		   mirrored, as escape, so it can pause a race. */
		static bool gamepad_menu_navigation;

		int n_joysticks;

		int joystick_0_pos, joystick_size; /* start of the first joystick and joystick size */
		int LVK_ID;

		int k_size;       /* number of inputs */
		unsigned char *old_keyboard;
		unsigned char *keyboard;
		List<int> keyevents; /* printable characters typed since the last cycle (from text input) */
};

#endif
