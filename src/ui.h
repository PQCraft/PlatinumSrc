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
    int event;
    psrc_ui_coord pos;
} psrc_ui_event;

typedef struct {
    char* id;
    int type;
    psrc_ui_coord pos;
    psrc_ui_coord size;
    int border;
    void* data;
    void* outdata;
} psrc_ui_elem;

typedef struct {
    unsigned id;
    psrc_ui_coord pos;
    psrc_ui_coord size;
    bool tbar;
    char* title;
    bool logo;
    uint8_t btns;
    uint8_t cbtns;
    int elemct;
    psrc_ui_elem* elems;
    void (*callback)(void*, psrc_ui_elem*, psrc_ui_event);
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
    psrc_ui_dialog* (*getDialog)(uint16_t);
    psrc_ui_elem* (*getElem)(psrc_ui_dialog*, char*);
    bool shown;
    double scale;
} psrc_ui_struct;

typedef struct {
    int linect;
    char** lines;
} psrc_ui_elem_list;

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
    PSRC_UI_BTN_CLOSE = 1,
    PSRC_UI_BTN_RESIZE = 2,
    PSRC_UI_BTN_HELP = 4,
};

enum {
    PSRC_UI_ELEM_BTN,
    PSRC_UI_ELEM_TBOX,
    PSRC_UI_ELEM_PBAR,
    PSRC_UI_ELEM_SLIDER,
    PSRC_UI_ELEM_CBOX,
    PSRC_UI_ELEM_RBTN,
    PSRC_UI_ELEM_LIST,
    PSRC_UI_ELEM_SKIP = 255,
};

enum {
    PSRC_UI_BDR_NONE,
    PSRC_UI_BDR_CONVEX,
    PSRC_UI_BDR_CONCAVE,
    PSRC_UI_BDR_SOLID,
};

enum {
    PSRC_UI_MOD_BTNS,
    PSRC_UI_MOD_ELEM,
    PSRC_UI_MOD_ADD,
    PSRC_UI_MOD_MOD,
    PSRC_UI_MOD_REMOVE,
    PSRC_UI_MOD_POS,
    PSRC_UI_MOD_SIZE,
    PSRC_UI_MOD_CALLBACK,
    PSRC_UI_MOD_BORDER,
    PSRC_UI_MOD_DATA,
};

enum {
    PSRC_UI_EVENT_CLOSE,
    PSRC_UI_EVENT_RESIZE,
    PSRC_UI_EVENT_HELP,
    PSRC_UI_EVENT_CLICK,
    PSRC_UI_EVENT_HOLD,
    PSRC_UI_EVENT_RELEASE,
    PSRC_UI_EVENT_SUBMIT,
    PSRC_UI_EVENT_MOVE,
};

#endif
