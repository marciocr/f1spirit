#ifndef __F1SPIRIT_POSTFX_H
#define __F1SPIRIT_POSTFX_H

/* Upscaling filters (VIDEO -> FILTER). With a filter on, the game is drawn
   into a 640x480 framebuffer, the filter shader(s) in shaders/ scale it 2x,
   and the result is stretched (bilinear, letterboxed) to the window. With
   FILTER_OFF the game draws straight to the window at its full resolution,
   as before. */

enum {
	FILTER_OFF = 0,
	FILTER_SCALE2X,
	FILTER_2XSAI,
	FILTER_HQ2X,
	FILTER_XBR,
	N_FILTERS
};

extern int video_filter;	/* persisted in f1spirit.cfg */

const char *postfx_filter_name(int filter);

/* Selects a filter, loading its shaders on first use. Falls back to
   FILTER_OFF (and returns false) if the GPU can't run it. */
bool postfx_set_filter(int filter);
bool postfx_active(void);

/* Around each frame: begin redirects the game's drawing into the 640x480
   framebuffer, end runs the filter and draws the result to the window. */
void postfx_begin_frame(void);
void postfx_end_frame(int window_w, int window_h);

void postfx_release(void);

#endif
