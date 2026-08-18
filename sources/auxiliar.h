#ifndef MOG_AUXILIAR
#define MOG_AUXILIAR

#ifndef M_PI
#define M_PI 3.141592654F
#endif

#ifndef _WIN32
#define min(a,b) ((a)<(b) ? (a) : (b))
#define max(a,b) ((a)>(b) ? (a) : (b))

#endif
//#define abs(a) ((a)<0 ? (-(a)) : (a))


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
// PPC values:
#define RMASK  0xff000000
#define GMASK  0x00ff0000
#define BMASK  0x0000ff00
#define AMASK  0x000000ff
#define AOFFSET 3
#define BOFFSET 2
#define GOFFSET 1
#define ROFFSET 0

#else
// Intel values:
#define RMASK  0x000000ff
#define GMASK  0x0000ff00
#define BMASK  0x00ff0000
#define AMASK  0xff000000
#define AOFFSET 3
#define BOFFSET 2
#define GOFFSET 1
#define ROFFSET 0

#endif

/* SDL3 compatibility shims.
   SDL1.2's SDL_CreateRGBSurface/SDL_SetAlpha/SDL_MapRGB(A)/SDL_GetRGBA all changed
   shape in SDL3 (surface flags are gone, per-surface alpha is now split into a blend
   mode plus an alpha modulation value, and a surface's pixel format is an enum instead
   of a struct pointer). These wrappers let every existing call site in this codebase
   keep using the SDL1.2 call shape unchanged. */
#define SDL_SWSURFACE 0
#define SDL_SRCALPHA 0x00010000
/* SDL3 has no hardware/software surface distinction any more; every surface we
   create goes through the shim above, so this flag never actually gets set — the
   define just keeps `surface->flags & SDL_HWSURFACE` checks compiling as "false". */
#define SDL_HWSURFACE 0

static inline SDL_Surface *F1S_CreateRGBSurface(Uint32 flags, int w, int h, int depth, Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask)
{
	(void)flags;
	return SDL_CreateSurface(w, h, SDL_GetPixelFormatForMasks(depth, rmask, gmask, bmask, amask));
}
#define SDL_CreateRGBSurface(flags, w, h, depth, rmask, gmask, bmask, amask) \
	F1S_CreateRGBSurface(flags, w, h, depth, rmask, gmask, bmask, amask)

static inline void F1S_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
	SDL_SetSurfaceBlendMode(surface, (flag & SDL_SRCALPHA) ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	/* SDL1.2 only honored `alpha` when SDL_SRCALPHA was set in `flag`; unlike SDL1.2,
	   SDL3's SDL_BlitSurface still applies the alpha modulation set here even under
	   SDL_BLENDMODE_NONE, so forwarding it unconditionally was baking alpha=0 into
	   every texture loaded via the many `SDL_SetAlpha(sfc, 0, 0)` call sites in this
	   codebase (flag=0, alpha meant to be a don't-care) — every draw using GL_BLEND
	   with a texture prepared this way was fully invisible. */
	if (flag & SDL_SRCALPHA)
		SDL_SetSurfaceAlphaMod(surface, alpha);
}
#define SDL_SetAlpha(surface, flag, alpha) F1S_SetAlpha(surface, flag, alpha)

static inline Uint32 F1S_MapRGB(SDL_PixelFormat format, Uint8 r, Uint8 g, Uint8 b)
{
	return SDL_MapRGB(SDL_GetPixelFormatDetails(format), 0, r, g, b);
}
#define SDL_MapRGB(format, r, g, b) F1S_MapRGB(format, r, g, b)

static inline Uint32 F1S_MapRGBA(SDL_PixelFormat format, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return SDL_MapRGBA(SDL_GetPixelFormatDetails(format), 0, r, g, b, a);
}
#define SDL_MapRGBA(format, r, g, b, a) F1S_MapRGBA(format, r, g, b, a)

static inline void F1S_GetRGBA(Uint32 pixel, SDL_PixelFormat format, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(format), 0, r, g, b, a);
}
#define SDL_GetRGBA(pixel, format, r, g, b, a) F1S_GetRGBA(pixel, format, r, g, b, a)

#define SDL_GetClipRect(surface, rect) SDL_GetSurfaceClipRect(surface, rect)
#define SDL_FillRect(dst, rect, color) SDL_FillSurfaceRect(dst, rect, color)
#define SDL_FreeSurface(surface) SDL_DestroySurface(surface)
/* SDL3_mixer's redesigned API dropped the old 0-128 volume scale in favor of
   a float gain (1.0 = normal); Sound_play()/Sound_music_volume() in sound.cpp
   still take the old 0-128 ints, so this keeps callers using the old MIX_MAX_VOLUME
   constant (the "full volume" value) unchanged. */
#define MIX_MAX_VOLUME 128

/* SDL3 dropped SDL_NumJoysticks(); the closest equivalent is asking
   SDL_GetJoysticks() for the array it allocates just to read its count back. */
static inline int SDL_NumJoysticks(void)
{
	int count = 0;
	SDL_JoystickID *ids = SDL_GetJoysticks(&count);
	SDL_free(ids);
	return count;
}

#ifndef _WIN32
#ifndef HAVE_STRLWR

char *strlwr (char *str);

#endif
#endif

bool substr(char *str, char *substr);
void pause(unsigned int time);


void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void maximumpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
Uint32 getpixel(SDL_Surface *surface, int x, int y);
void draw_rectangle(SDL_Surface *surface, int x, int y, int w, int h, Uint32 pixel);
void draw_rectangle_vp(SDL_Surface *surface, int x, int y, int w, int h, Uint32 pixel, SDL_Rect *vp);
void draw_line(SDL_Surface *SDL, int x1, int y1, int x2, int y2, Uint32 pixel);
void draw_thick_line(SDL_Surface *SDL, int x1, int y1, int x2, int y2, Uint32 pixel, int thickness);
void area_fill(SDL_Surface *SDL, int x, int y, Uint32 pixel);
void fast_area_fill(SDL_Surface *SDL, int x, int y, Uint32 pixel);

void surface_shifter(SDL_Surface *surface, int r_inc, int g_int, int b_inc, int a_inc, SDL_Rect *r);
void surface_fader(SDL_Surface *surface, float r_factor, float g_factor, float b_factor, float a_factor, SDL_Rect *r);
void surface_shader(SDL_Surface *surface, float factor, int red, int green, int blue, int alpha, SDL_Rect *r);
void surface_bicolor(SDL_Surface *surface, float factor, int r1, int g1, int b1, int a1, int r2, int g2, int b2, int a2, SDL_Rect *r);

void surface_fader_mask(SDL_Surface *surface, SDL_Surface *mask, float r_factor, float g_factor, float b_factor, float a_factor, SDL_Rect *r);


SDL_Surface *load_maskedimage(char *image, char *mask, char *path);

void surface_automatic_alpha(SDL_Surface *sfc);
void surface_bw(SDL_Surface *sfc, int threshold);
void surface_mask_from_bitmap(SDL_Surface *sfc, SDL_Surface *mask, int x, int y);

bool segment_collision(int s1x1, int s1y1, int s1x2, int s1y2,
                       int s2x1, int s2y1, int s2x2, int s2y2,
                       int *cx, int *cy);

/*
void print_left(char *text,TTF_Font *font,int r,int g,int b,SDL_Surface *sfc,int x,int y);
void print_centered(char *text,TTF_Font *font,int r,int g,int b,SDL_Surface *sfc,int x,int y);
int get_text_width(char *text, TTF_Font *font);

SDL_Surface *multiline_text_surface(char *text,int line_dist,TTF_Font *font,SDL_Color c);
SDL_Surface *multiline_text_surface_limited(char *text,int line_dist,TTF_Font *font,SDL_Color c,int max_w);

*/

void print_left_bmp(unsigned char *text, SDL_Surface *font, SDL_Surface *sfc, int x, int y, int spacing);
void print_centered_bmp(unsigned char *text, SDL_Surface *font, SDL_Surface *sfc, int x, int y, int spacing);
int get_text_width_bmp(unsigned char *text, SDL_Surface *font, int spacing);

/* Blits orig into dest, and sets the alpha channel of all the blitted pixels to the right value: */
void surface_blit_alpha(SDL_Surface *orig, SDL_Rect *o_r, SDL_Surface *dest, SDL_Rect *d_r);

/* functions to write/read from files: */

bool save_float(FILE *fp, float f);
bool load_float(FILE *fp, float *f);

bool save_int(FILE *fp, int f);
bool load_int(FILE *fp, int *f);


#endif

