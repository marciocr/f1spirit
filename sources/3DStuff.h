#ifndef __3DSTUFF_H_
#define __3DSTUFF_H_

#include <GL/gl.h>
//#include "GL/glu.h"

extern int MINX;
extern int MAXX;
extern int MINY;
extern int MAXY;

extern int desktopW;
extern int desktopH;

extern int SCREENW;
extern int SCREENH;
extern int SCREENX;
extern int SCREENY;

extern float screenScale;

void calcMinMax(int width, int height);

#endif //__3DSTUFF_H_