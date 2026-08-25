#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "3DStuff.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_image/SDL_image.h>

#include "F1Spirit.h"
#include "sound.h"
#include "List.h"
#include "2DCMC.h"
#include "auxiliar.h"
#include "GLTile.h"
#include "PlacedGLTile.h"
#include "RotatedGLTile.h"
#include "keyboardstate.h"
#include "2DCMC.h"
#include "RoadPiece.h"
#include "track.h"
#include "CPlayer.h"
#include "CCar.h"
#include "RacingCCar.h"
#include "PlayerCCar.h"
#include "EnemyCCar.h"
#include "GameParameters.h"
#include "ReplayInfo.h"
#include "F1SpiritGame.h"
#include "F1SpiritApp.h"
#include "F1Spirit-auxiliar.h"
#include "randomc.h"

#include "debug.h"


#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif

/*      GLOBAL VARIABLES INITIALIZATION:       */

char *application_name = "F-1 Spirit";
int application_version = 0;
int SCREEN_X = 640;
int SCREEN_Y = 480;
int g_stencil_bits = 0;
int N_SFX_CHANNELS = 16;
int COLOUR_DEPTH = 32;
bool sound = true;
bool fullscreen = false;

SDL_Window *main_window = 0;
SDL_GLContext gl_context = 0;

TRanrotBGenerator *rg = 0;

const int REDRAWING_PERIOD = 20;	// => 20 ms = 50 fps

/* Frames per second counter: */
int frames_per_sec = 0;
int frames_per_sec_tmp = 0;
int init_time = 0;
bool show_fps = false;

/* F1Spirit console messages: */
bool show_console_msg = false;
char console_msg[80] = "";


/*      AUXILIAR FUNCTION DEFINITION:       */


bool initialization(Uint64 flags)
{
	rg = new TRanrotBGenerator(0);

#ifdef F1SPIRIT_DEBUG_MESSAGES

	output_debug_message("Initializing SDL\n");
#endif

	if (!SDL_Init(SDL_INIT_VIDEO | (sound ? SDL_INIT_AUDIO : 0) | SDL_INIT_GAMEPAD)) {
#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("Video initialization failed: %s\n", SDL_GetError());
#endif

		return false;
	}

#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("SDL initialized\n");

#endif

	const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());

	if (!mode) {
#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("Video query failed: %s\n", SDL_GetError());
#endif

		return false;
	}

	desktopW = mode->w;
	desktopH = mode->h;
#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("Setting OpenGL attributes\n");

#endif
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("OpenGL attributes set\n");

#endif

#ifdef F1SPIRIT_DEBUG_MESSAGES

	output_debug_message("Initializing video mode\n");

#endif
	flags = SDL_WINDOW_OPENGL | flags;
	main_window = SDL_CreateWindow(application_name, (fullscreen)?desktopW:SCREEN_X, (fullscreen)?desktopH:SCREEN_Y, flags);

	if (main_window == 0) {
#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("Window creation failed: %s\n", SDL_GetError());
#endif

		return false;
	}

	gl_context = SDL_GL_CreateContext(main_window);

	if (gl_context == 0) {
#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("GL context creation failed: %s\n", SDL_GetError());
#endif

		return false;
	}

#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("Video mode initialized\n");

#endif

	calcMinMax((fullscreen)?desktopW:SCREEN_X, (fullscreen)?desktopH:SCREEN_Y);

	SDL_SetWindowTitle(main_window, application_name);

	SDL_SetWindowIcon(main_window, SDL_LoadBMP("graphics/f1sicon.bmp"));

	SDL_HideCursor();

	if (sound) {
#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("Initializing Audio\n");
#endif

		N_SFX_CHANNELS = Sound_initialization(N_SFX_CHANNELS, 0);

#ifdef F1SPIRIT_DEBUG_MESSAGES

		output_debug_message("Audio initialized\n");
#endif

	}

	SDL_StartTextInput(main_window);

	glGetIntegerv(GL_STENCIL_BITS, &g_stencil_bits);

#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("OpenGL stencil buffer bits: %i\n", g_stencil_bits);

#endif


	return true;
} /* initialization */


void finalization()
{
#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("Finalizing SDL\n");
#endif

	delete rg;

	rg = 0;

	free_auxiliar_menu_surfaces();

	if (sound)
		Sound_release();

	SDL_Quit();

#ifdef F1SPIRIT_DEBUG_MESSAGES

	output_debug_message("SDL finalized\n");

#endif

} /* finalization */



#ifdef _WIN32
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(int argc, char** argv) {
#endif

	F1SpiritApp *game;
	KEYBOARDSTATE *k;

	Uint64 time, act_time;
	SDL_Event event;
	bool quit = false;
	bool need_to_redraw = true;

#ifdef F1SPIRIT_DEBUG_MESSAGES

	output_debug_message("Application started\n");
#endif
	if (!initialization((fullscreen ? SDL_WINDOW_FULLSCREEN : 0)))
		return 0;

	k = new KEYBOARDSTATE();

	game = new F1SpiritApp();

	time = init_time = SDL_GetTicks();

	while (!quit) {
		while ( SDL_PollEvent( &event ) ) {
			switch ( event.type ) {
					/* Keyboard event */

				case SDL_EVENT_KEY_DOWN:
#ifdef __APPLE__

					if (event.key.scancode == SDL_SCANCODE_Q) {
						SDL_Keymod modifiers;
						modifiers = SDL_GetModState();

						if ((modifiers & SDL_KMOD_GUI) != 0) {
							quit = true;
						}
					}

#else
					if (event.key.scancode == SDL_SCANCODE_F12) {
						quit = true;
					}

#endif
					if (event.key.scancode == SDL_SCANCODE_F10) {
						game->save_configuration("f1spirit.cfg");
						game->load_configuration("f1spirit.cfg");
					}

#ifdef _WIN32
					if (event.key.scancode == SDL_SCANCODE_F4) {
						SDL_Keymod modifiers;

						modifiers = SDL_GetModState();

						if ((modifiers & SDL_KMOD_ALT) != 0)
							quit = true;
					}

#endif
#ifdef __APPLE__
					if (event.key.scancode == SDL_SCANCODE_F) {
						SDL_Keymod modifiers;

						modifiers = SDL_GetModState();

						if ((modifiers & SDL_KMOD_GUI) != 0) {
							/* Toggle FULLSCREEN mode: */
							if (fullscreen)
								fullscreen = false;
							else
								fullscreen = true;

							SDL_SetWindowFullscreen(main_window, fullscreen);

							calcMinMax((fullscreen)?desktopW:SCREEN_X, (fullscreen)?desktopH:SCREEN_Y);

							SDL_HideCursor();

							reload_textures++;
						}
					}

#else
					if (event.key.scancode == SDL_SCANCODE_RETURN) {
						SDL_Keymod modifiers;

						modifiers = SDL_GetModState();

						if ((modifiers & SDL_KMOD_ALT) != 0) {
							/* Toggle FULLSCREEN mode: */
							if (fullscreen)
								fullscreen = false;
							else
								fullscreen = true;

							SDL_SetWindowFullscreen(main_window, fullscreen);

							calcMinMax((fullscreen)?desktopW:SCREEN_X, (fullscreen)?desktopH:SCREEN_Y);

							SDL_HideCursor();

							reload_textures++;
						}
					}

#endif

					if (event.key.scancode == SDL_SCANCODE_F) {
						SDL_Keymod modifiers;

						modifiers = SDL_GetModState();

						if ((modifiers & SDL_KMOD_ALT) != 0) {
							/* toggle FPS mode: */
							if (show_fps)
								show_fps = false;
							else
								show_fps = true;
						}
					}

					break;

					/* Printable text typed by the user (player name entry, etc): fed
					   into the KEYBOARDSTATE text-edit queue, ASCII bytes only. */

				case SDL_EVENT_TEXT_INPUT: {
						const char *text = event.text.text;
						int i;

						for (i = 0;text[i] != 0;i++) {
							if ((unsigned char)text[i] < 0x80)
								k->keyevents.Add(new int(text[i]));
						}
					}

					break;

					/* SDL_EVENT_QUIT (window close) */

				case SDL_EVENT_QUIT:
					quit = true;

					break;
			}
		}

		act_time = SDL_GetTicks();

		if (act_time - time >= (Uint64)REDRAWING_PERIOD)
		{
			int max_frame_step = 10;
			/*
			   frames_per_sec_tmp+=1;
			   if ((act_time-init_time)>=1000) {
			    frames_per_sec=frames_per_sec_tmp;
			    frames_per_sec_tmp=0;
			    init_time=act_time;
			   } // if
			*/
			int min_frame=1;

			do {
				time += REDRAWING_PERIOD;

				if ((act_time - time) > (Uint64)(10*REDRAWING_PERIOD))
					time = act_time;

				/* cycle */
				k->cycle();

				if (!game->cycle(k))
					quit = true;

				need_to_redraw = true;

				k->keyevents.Delete();

				act_time = SDL_GetTicks();

				max_frame_step--;
				min_frame--;
			} while (((act_time - time >= (Uint64)REDRAWING_PERIOD) && (max_frame_step > 0)) || (min_frame > 0));

		}

		/* Redraw */
		if (need_to_redraw) {
			game->draw();
			need_to_redraw = false;
			frames_per_sec_tmp += 1;
		}

		if ((act_time - (Uint64)init_time) >= 1000) {
			frames_per_sec = frames_per_sec_tmp;
			frames_per_sec_tmp = 0;
			init_time = act_time;
		}

		SDL_Delay(1);

	}


	delete k;

	k = 0;

	delete game;

	game = 0;

	Stop_playback();

	finalization();

#ifdef F1SPIRIT_DEBUG_MESSAGES

	output_debug_message("Application finished\n");

	close_debug_messages();

#endif

	return 0;
} /* main */
