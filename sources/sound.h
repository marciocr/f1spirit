#ifndef __BRAIN_SDL_SOUND
#define __BRAIN_SDL_SOUND

typedef MIX_Audio * SOUNDT;

bool Sound_initialization(void);
int Sound_initialization(int nc, int nrc);
void Sound_release(void);

bool Sound_file_test(char *f1);

SOUNDT Sound_create_sound(char *file);
void Sound_delete_sound(SOUNDT s);
void Sound_play(SOUNDT s);
void Sound_play(SOUNDT s, int volume);
void Sound_play_ch(SOUNDT s, int channel);
void Sound_play_ch(SOUNDT s, int channel, int volume);

void Sound_create_music(char *f1, int times);
void Sound_release_music(void);
void Sound_pause_music(void);
void Sound_unpause_music(void);

void Sound_music_volume(int volume);

/* Master volumes in percent (0-100), set from the AUDIO menu. Sound_sfx_gain()
   is the factor for tracks started directly through MIX_PlayTrack(). */
void Sound_set_master_volume(int music, int sfx);
int Sound_get_master_music_volume(void);
int Sound_get_master_sfx_volume(void);
float Sound_sfx_gain(void);

/* These functions are AGRESIVE! (i.e. they actually STOP SDL_mixer and restart it) */
void Stop_playback(void);
void Resume_playback(void);
int Resume_playback(int nc, int nr);

/* Path (with extension) that was actually opened by the last successful
   Sound_create_sound() call. Used by CarEngineSound.cpp to independently
   decode the engine sample into a raw, randomly-indexable PCM buffer for
   its pitch-shifting effect (MIX_Audio itself does not expose raw samples). */
char *Sound_last_loaded_path(void);

/* The mixer track backing a given channel index, so callers that need
   lower-level control (per-channel effect callbacks, explicit pause/stop)
   can drive SDL3_mixer's MIX_Track API directly, the way the original code
   drove SDL_mixer channels directly. */
struct MIX_Track *Sound_get_channel_track(int channel);

#endif

