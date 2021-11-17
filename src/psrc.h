#ifndef PSRC

#define PSRC 2021111702

#define MACSTR_IND(x) #x
#define MACSTR(x) MACSTR_IND(x)

#define PSRC_STR "PlatinumSrc build "MACSTR(PSRC)

#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>

#include "sound.h"
#include "gfx.h"
#include "main.h"

#ifdef _WIN32
#include <windows.h>
#endif

#endif
