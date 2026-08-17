#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "sound.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"

#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif


bool sound_enabled = false;
static MIX_Mixer *mixer = 0;
static MIX_Audio *music_sound = 0;
static MIX_Track *music_track = 0;
static MIX_Track **channel_tracks = 0;
static int n_channels = -1;
static int n_reserved_channels = 0;
static char last_loaded_path[256] = "";


static MIX_Track *find_free_channel(void)
{
	int i;

	for (i = n_reserved_channels;i < n_channels;i++) {
		if (!MIX_TrackPlaying(channel_tracks[i]))
			return channel_tracks[i];
	}

	return (n_channels > n_reserved_channels) ? channel_tracks[n_reserved_channels] : 0;
} /* find_free_channel */


bool Sound_initialization(void)
{
	if ( -1 == Sound_initialization(0, 0))
		return false;

	return true;
} /* Sound_initialization */


int Sound_initialization(int nc, int nrc)
{
	int i;

	sound_enabled = true;
#ifdef F1SPIRIT_DEBUG_MESSAGES

	output_debug_message("Initializing SDL_mixer.\n");
#endif

	if (!MIX_Init()) {
		sound_enabled = false;
#ifdef F1SPIRIT_DEBUG_MESSAGES

		output_debug_message("Unable to initialize SDL_mixer: %s\n", SDL_GetError());
		output_debug_message("Running the game without audio.\n");
#endif

		return -1;
	}

	mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 0);

	if (mixer == 0) {
		sound_enabled = false;
#ifdef F1SPIRIT_DEBUG_MESSAGES

		output_debug_message("Unable to open audio: %s\n", SDL_GetError());
		output_debug_message("Running the game without audio.\n");
#endif

		MIX_Quit();

		return -1;
	}

	music_track = MIX_CreateTrack(mixer);

	n_channels = (nc > 0) ? nc : 8;

	channel_tracks = new MIX_Track * [n_channels];

	for (i = 0;i < n_channels;i++)
		channel_tracks[i] = MIX_CreateTrack(mixer);

	n_reserved_channels = (nrc > 0) ? nrc : 0;

	return n_channels;
} /* Sound_init */

void Sound_release(void)
{
	int i;

	Sound_release_music();

	if (sound_enabled) {
		if (channel_tracks != 0) {
			for (i = 0;i < n_channels;i++)
				MIX_DestroyTrack(channel_tracks[i]);

			delete []channel_tracks;

			channel_tracks = 0;
		}

		if (music_track != 0) {
			MIX_DestroyTrack(music_track);
			music_track = 0;
		}

		if (mixer != 0) {
			MIX_DestroyMixer(mixer);
			mixer = 0;
		}

		MIX_Quit();
	}

	sound_enabled = false;
} /* Sound_release */


void Stop_playback(void)
{
	Sound_release();
} /* Stop_playback */

void Resume_playback(void)
{
	Resume_playback(0, 0);
} /* Resume_playback */


int Resume_playback(int nc, int nrc)
{
	return Sound_initialization(nc, nrc);
} /* Resume_playback */


/* a check to see if file is readable and greater than zero */
int file_check(char *fname)
{
	FILE *fp;

	if ((fp = fopen(fname, "r")) != NULL) {
		if (fseek(fp, 0L, SEEK_END) == 0 && ftell(fp) > 0) {
			fclose(fp);
			return true;
		}

		/* either the file could not be read (==-1) or size was zero (==0) */
#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("ERROR in file_check(): the file %s is corrupted.\n", fname);

#endif

		fclose(fp);

		exit(1);
	}

	return false;
} /* file_check */



SOUNDT Sound_create_sound(char *file)
{
	int n_ext = 6;
	char *ext[6] = {".ogg", ".wav", ".mp3", ".OGG", ".WAV", ".MP3"};
	char name[256];
	int i;

	if (sound_enabled) {
		for (i = 0;i < n_ext;i++) {
			strcpy(name, file);
			strcat(name, ext[i]);

			if (file_check(name)) {
				strcpy(last_loaded_path, name);
				return MIX_LoadAudio(mixer, name, false);
			}
		}

#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("ERROR in Sound_create_sound(): Could not load sound file: %s.(wav|ogg|mp3)\n", file);

#endif

		exit(1);
	} else {
		return 0;
	}
} /* Sound_create_sound */


char *Sound_last_loaded_path(void)
{
	return last_loaded_path;
} /* Sound_last_loaded_path */


MIX_Track *Sound_get_channel_track(int channel)
{
	if (channel_tracks == 0 || channel < 0 || channel >= n_channels)
		return 0;

	return channel_tracks[channel];
} /* Sound_get_channel_track */


void Sound_delete_sound(SOUNDT s)
{
	if (sound_enabled)
		MIX_DestroyAudio(s);
} /* Sound_delete_sound */


void Sound_play(SOUNDT s)
{
	Sound_play(s, 128);
} /* Sound_play */


void Sound_play(SOUNDT s, int volume)
{
	MIX_Track *t;

	if (sound_enabled) {
		t = find_free_channel();

		if (t != 0) {
			MIX_SetTrackAudio(t, s);
			MIX_SetTrackLoops(t, 0);
			MIX_SetTrackGain(t, volume / 128.0F);
			MIX_PlayTrack(t, 0);
		}
	}
} /* Sound_play */

void Sound_play_ch(SOUNDT s, int ch)
{
	Sound_play_ch(s, ch, 128);
} /* Sound_play_ch */


void Sound_play_ch(SOUNDT s, int ch, int volume)
{
	MIX_Track *t;

	if (sound_enabled && ch >= 0 && ch < n_channels) {
		t = channel_tracks[ch];
		MIX_SetTrackAudio(t, s);
		MIX_SetTrackLoops(t, 0);
		MIX_SetTrackGain(t, volume / 128.0F);
		MIX_PlayTrack(t, 0);
	}
} /* Sound_play_ch */

MIX_Audio *Sound_create_stream(char *file)
{
	int n_ext = 6;
	char *ext[6] = {".ogg", ".wav", ".mp3", ".OGG", ".WAV", ".MP3"};
	char name[256];
	int i;

	if (sound_enabled) {
		for (i = 0;i < n_ext;i++) {
			strcpy(name, file);
			strcat(name, ext[i]);

			if (file_check(name))
				return MIX_LoadAudio(mixer, name, false);
		}

#ifdef F1SPIRIT_DEBUG_MESSAGES
		output_debug_message("ERROR in Sound_create_stream(): Could not load sound file: %s.(wav|ogg|mp3)\n", file);

#endif

		exit(1);
	} else {
		return 0;
	}
} /* Sound_create_stream */


void Sound_create_music(char *f1, int loops)
{
	if (sound_enabled) {
		if (f1 != 0) {
			if (music_sound != 0)
				MIX_DestroyAudio(music_sound);

			music_sound = Sound_create_stream(f1);
			MIX_SetTrackAudio(music_track, music_sound);
			MIX_SetTrackLoops(music_track, loops);
			MIX_PlayTrack(music_track, 0);
		} else {
			music_sound = 0;
		}

		//  playing_music=true;
	}
} /* Sound_create_music */


bool Sound_file_test(char *f1)
{
	int n_ext = 6;
	char *ext[6] = {".WAV", ".OGG", ".MP3", ".wav", ".ogg", ".mp3"};
	char name[256];
	int i;

	if (sound_enabled) {
		for (i = 0;i < n_ext;i++) {
			strcpy(name, f1);
			strcat(name, ext[i]);

			if (file_check(name))
				return true;
		}

		return false;
	} else {
		return false;
	}
} /* Sound_file_test */


void Sound_release_music(void)
{
	if (sound_enabled) {
		//  playing_music=false;
		if (music_track != 0)
			MIX_StopTrack(music_track, 0);

		if (music_sound != 0)
			MIX_DestroyAudio(music_sound);

		music_sound = 0;
	}
} /* Sound_release_music */



void Sound_pause_music(void)
{
	if (music_track != 0)
		MIX_PauseTrack(music_track);
} /* Sound_pause_music */


void Sound_unpause_music(void)
{
	if (music_track != 0)
		MIX_ResumeTrack(music_track);
} /* Sound_unpause_music */


void Sound_music_volume(int volume)
{
	if (volume < 0)
		volume = 0;

	if (volume > 127)
		volume = 127;

	if (music_track != 0)
		MIX_SetTrackGain(music_track, volume / 128.0F);
} /* Sound_music_volume */
