#ifndef PSRC_COMMON_COMMON_H
#define PSRC_COMMON_COMMON_H

#include "config.h"

struct options {
    char* game;
    char* mods;
    char* icon;
    struct cfg* set;
    char* maindir;
    char* userdir;
    char* config;
    bool nouserconfig;
    bool nocontroller;
};

extern int quitreq;

extern char* maindir;
extern char* userdir;

extern char* gamedir; // relative to <maindir>/games
extern char* savedir;

extern struct cfg* config;
extern struct cfg* gameconfig;

extern struct options options;

#endif
