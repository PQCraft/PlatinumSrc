#ifndef PSRC_UI

#define PSRC_UI

#include <stdarg.h>

typedef struct {
    int x;
    int y;
} psrc_ui_coord;

typedef struct {
    unsigned int texid;
    psrc_ui_coord size;
    psrc_ui_coord bearing;
    unsigned int advance;
} psrc_ui_chardata;

typedef struct {
    int type;
    psrc_ui_coord pos;
    char** data;
} psrc_ui_elem;

typedef struct {
    char* title;
    bool closable;
    psrc_ui_elem* elems;
} psrc_ui_dialog;

typedef struct {
    void (*renderHook)(void);
    int (*newDialog)(char*, bool, ...);
    void (*modDialog)(int, ...);
    void (*closeDialog)(int);
    void (*pushToFront)(int);
    void (*pushToBack)(int);
    void (*showUI)(void);
    void (*hideUI)(void);
    void (*deinit)(void);
    bool shown;
} psrc_ui_struct;

psrc_ui_struct* psrc_ui_init();

#endif
