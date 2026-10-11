#ifndef __F1SPIRIT_VIDEO_H
#define __F1SPIRIT_VIDEO_H

/* Video settings (VIDEO menu, Alt+Enter), persisted in f1spirit.cfg. */
extern bool fullscreen;
extern int window_scale;	/* windowed size = 640x480 * window_scale */

void video_set_fullscreen(bool f);
void video_set_window_scale(int scale);
int video_max_window_scale(void);	/* largest scale that fits the display */
void video_window_resized(void);	/* recompute the viewport from the window size */

#endif
