#ifndef PSRC_MAIN

#define PSRC_MAIN

#define PSRC_INF 0
#define PSRC_WRN 1
#define PSRC_ERR 2

#include <stdarg.h>

#include "sound.h"
#include "gfx.h"
#include "ui.h"

typedef struct {
    void (*displayError)(int, char*, char*);
    void (*wait)(uint64_t);
    uint64_t (*utime)(void);
    float (*randfloat)(float, float);
    char* (*getTextFile)(char*);
    char* (*getTextFileSilent)(char*);
    char* (*getFText)(char*, ...);
    void (*getCfgVar)(char*, char*, char*, char*);
    char* (*getCfgVarStatic)(char*, char*, char*);
    char* (*getCfgVarAlloc)(char*, char*, char*);
    bool (*cfgValBool)(char*);
    psrc_sound_struct* sound;
    psrc_gfx_struct* gfx;
    psrc_ui_struct* ui;
    bool quitRequested;
} psrc_main_struct;

extern psrc_main_struct psrc;

#endif
