#ifdef _WIN32
#include "windows.h"
#endif

#include "math.h"
#include "stdlib.h"
#include "string.h"

#include "3DStuff.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "F1Spirit.h"
#include "sound.h"
#include "List.h"
#include "2DCMC.h"
#include "auxiliar.h"
#include "GLTile.h"
#include "PlacedGLTile.h"
#include "PlacedGLTile.h"
#include "RotatedGLTile.h"
#include "keyboardstate.h"
#include "CPlayer.h"
#include "RoadPiece.h"
#include "CCar.h"
#include "2DCMC.h"
#include "track.h"
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

extern bool sound;

/* carEngineSoundEffect() needs to freely index into the *entire* engine
   sample at an arbitrary, wrapping offset (to fake a pitch change by varying
   the read speed). MIX_Audio doesn't expose its decoded samples directly, so
   we keep our own persistent, fully-decoded copy alongside it, decoded once
   from the same file Sound_create_sound() loaded for S_car_engine. */

static float *engine_pcm = 0;
static int engine_pcm_frames = 0;
static int engine_pcm_channels = 0;

void carEngineSoundEffect_load(void)
{
	MIX_AudioDecoder *dec;
	SDL_AudioSpec spec;
	int cap, got, total;

	if (engine_pcm != 0) {
		delete []engine_pcm;
		engine_pcm = 0;
	}

	engine_pcm_frames = 0;
	engine_pcm_channels = 0;

	dec = MIX_CreateAudioDecoder(Sound_last_loaded_path(), 0);

	if (dec == 0)
		return;

	MIX_GetAudioDecoderFormat(dec, &spec);

	spec.format = SDL_AUDIO_F32;

	cap = spec.freq * spec.channels * 20; /* 20 seconds worth, grown below if needed */
	engine_pcm = new float[cap];
	total = 0;

	for (;;) {
		if (total + spec.freq * spec.channels > cap) {
			int newcap = cap * 2;
			float *tmp = new float[newcap];
			memcpy(tmp, engine_pcm, total * sizeof(float));
			delete []engine_pcm;
			engine_pcm = tmp;
			cap = newcap;
		}

		got = MIX_DecodeAudio(dec, ((Uint8 *)engine_pcm) + total * sizeof(float), (cap - total) * (int)sizeof(float), &spec);

		if (got <= 0)
			break;

		total += got / sizeof(float);
	}

	MIX_DestroyAudioDecoder(dec);

	engine_pcm_channels = spec.channels;
	engine_pcm_frames = (engine_pcm_channels > 0) ? total / engine_pcm_channels : 0;
} /* carEngineSoundEffect_load */


void carEngineSoundEffect(void *userdata, struct MIX_Track *track, const SDL_AudioSpec *spec, float *pcm, int samples)
{
	CCar *car = (CCar *)userdata;

	float f;
	int i, k;
	int channels = spec->channels;

#ifdef F1SPIRIT_DEBUG_MESSAGES
	output_debug_message("carEngineSoundEffect\n");
#endif

	if (engine_pcm == 0 || engine_pcm_frames <= 0) {
		memset(pcm, 0, (size_t)samples * channels * sizeof(float));
		return;
	}

	if (car->get_state() != 2) {
		/* state = 2 is when the car has crashed, thus we don't have to play the engine: */

		/* "f" is the factor at which we are modifying the pitch of the engine sample
		   if "f=2", the sound will be with twice the pitch of the original engine sample.
		   the formula we are using is: f = ((rpm-2000)/2000)+0.5 */
		f = (car->get_rpm() - 2000.0F) / 2000.0F;

		if (f < 0)
			f = 0;

		f = 0.5F + f;

		for (i = 0;i < samples;i++) {
			k = car->carengine_position + (int)(i * f);

			while (k >= engine_pcm_frames)
				k -= engine_pcm_frames;

			if (engine_pcm_channels >= 2) {
				pcm[i*channels + 0] = engine_pcm[k * engine_pcm_channels + 0];

				if (channels >= 2)
					pcm[i*channels + 1] = engine_pcm[k * engine_pcm_channels + 1];
			} else {
				int c;

				for (c = 0;c < channels;c++)
					pcm[i*channels + c] = engine_pcm[k];
			}
		}

		car->carengine_position = k;
	} else {
		/* When the car has crashed, just set the buffer to silence: */
		memset(pcm, 0, (size_t)samples * channels * sizeof(float));
		car->carengine_position = 0;
	} /* if */
}
