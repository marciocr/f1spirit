#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STRLEN 512
#define GAMENAME "f1spirit"

enum filetype { GAMEDATA, USERDATA };

#ifndef _WIN32
int mkdirp(const char *fqfn, mode_t mode);
#endif
FILE *f1open(const char *f, const char *m, const enum filetype t);

/* Full path of a game/user file, for operations that aren't fopen()
   (rename, remove, opendir). USERDATA files live in $HOME/.f1spirit/. */
char *f1path(const char *f, const enum filetype t, char *fname);

