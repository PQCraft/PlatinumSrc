#ifndef PSRC

#define PSRC 2021112501

#define PSRC_MACSTR_IND(x) #x
#define PSRC_MACSTR(x) PSRC_MACSTR_IND(x)

#define PSRC_STR "PlatinumSrc build "PSRC_MACSTR(PSRC)

#define PSRC_SWAP(a, b) {__typeof__(a) c = a; a = b; b = c;}

#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#endif
