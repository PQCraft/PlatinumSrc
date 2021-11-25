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
    char* id;
    int type;
    psrc_ui_coord pos;
    psrc_ui_coord size;
    int border;
    void* data;
    void* outdata;
    void (*callback)(char*, void*);
} psrc_ui_elem;

typedef struct {
    unsigned id;
    psrc_ui_coord pos;
    psrc_ui_coord size;
    bool tbar;
    char* title;
    bool logo;
    uint8_t btns;
    int elemct;
    void (*callback)(uint16_t, void*);
    psrc_ui_elem* elems;
} psrc_ui_dialog;

typedef struct {
    void (*renderHook)(void);
    void (*pollUI)(void);
    uint16_t (*newDialog)(int, int, int, int, bool, char*, bool, uint8_t, void*, int, ...);
    void (*modDialog)(uint16_t, ...);
    void (*closeDialog)(uint16_t);
    void (*pushToFront)(uint16_t);
    void (*pushToBack)(uint16_t);
    void (*loadUI)(char*, char*);
    void (*showUI)(void);
    void (*hideUI)(void);
    void (*deinit)(void);
    bool shown;
} psrc_ui_struct;

psrc_ui_struct* psrc_ui_init();

enum {
    PSRC_UI_WIN_FILL,
    PSRC_UI_WIN_LOGO,
    PSRC_UI_WIN_CLOSE,
    PSRC_UI_WIN_RESIZE,
    PSRC_UI_WIN_HELP,
    PSRC_UI_WIN_TBAR_LEFT,
    PSRC_UI_WIN_TBAR,
    PSRC_UI_WIN_TBAR_RIGHT,
    PSRC_UI_WIN_BDR_TOP,
    PSRC_UI_WIN_BDR_BTM,
    PSRC_UI_WIN_BDR_LEFT,
    PSRC_UI_WIN_BDR_RIGHT,
    PSRC_UI_WIN_BDR_SOLID,
    PSRC_UI_WIN_BTN,
    PSRC_UI_WIN_BTN_HOVER,
    PSRC_UI_WIN_BTN_PRESS,
    PSRC_UI_WIN_PBAR,
    PSRC_UI_WIN_SLD_LEFT,
    PSRC_UI_WIN_SLD,
    PSRC_UI_WIN_SLD_RIGHT,
    PSRC_UI_WIN_TBOX,
    PSRC_UI_WIN_CBOX_TRUE,
    PSRC_UI_WIN_CBOX_FALSE,
    PSRC_UI_WIN_RBTN_TRUE,
    PSRC_UI_WIN_RBTN_FALSE,
    PSRC_UI_WIN_DRPDN,
    PSRC_UI_WIN_LIST,
};

enum {
    PSRC_UI_ELEM_BTN,
    PSRC_UI_ELEM_TBOX,
    PSRC_UI_ELEM_PBAR,
    PSRC_UI_ELEM_SLIDER,
    PSRC_UI_ELEM_SKIP = 255,
};

enum {
    PSRC_UI_MOD_ELEM,
    PSRC_UI_MOD_BTNS,
    PSRC_UI_MOD_ADD,
    PSRC_UI_MOD_REMOVE,
    PSRC_UI_MOD_POS,
    PSRC_UI_MOD_SIZE,
    PSRC_UI_MOD_CALLBACK,
    PSRC_UI_MOD_DATA,
};

#define PSRC_UI_BTN_CLOSE 1
#define PSRC_UI_BTN_RESIZE 2
#define PSRC_UI_BTN_HELP 4

#endif
