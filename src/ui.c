#include "psrc.h"
#include "ui.h"
#include "gfx.h"
#include "main.h"

#include <ft2build.h>
#include FT_FREETYPE_H

psrc_ui_struct psrc_ui;
FT_Library psrc_ui_ftlib;
FT_Face psrc_ui_ftface;

void psrc_ui_deinit() {
    FT_Done_Face(psrc_ui_ftface);
    FT_Done_FreeType(psrc_ui_ftlib);
}

float psrc_ui_testvertices[] = {
     0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
     0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
};

unsigned int psrc_ui_testindices[] = {
    0, 1, 2, 2, 3, 0
};

psrc_gfx_obj* psrc_ui_elemobj[32];

void psrc_ui_renderObj2D(psrc_gfx_obj obj, int x, int y, int w, int h) {
    obj.pos = (psrc_coord_3d){(float)x * 2 / (float)psrc.gfx->cur_width - 1, 1 - (((float)y * 2 + (float)h * 2) / (float)psrc.gfx->cur_height), 0};
    obj.scale = (psrc_coord_3d){(float)w * 2 / (float)psrc.gfx->cur_width, (float)h * 2 / (float)psrc.gfx->cur_height, 1};
    psrc.gfx->renderObj(&obj);
}

void psrc_ui_loadElem(int elem, char* tex) {
    psrc_ui_elemobj[elem] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        psrc.getFText("resources/base/images/ui/%s", tex), 0, 1, false);
}

void psrc_ui_renderElem(int id, int x, int y, int w, int h) {
    psrc_ui_renderObj2D(*psrc_ui_elemobj[id], x, y, w, h);
}

void psrc_ui_renderBorder(int x, int y, int w, int h) {
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_TOP, x, y, w, 1);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_BTM, x, y + h - 1, w, 1);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_LEFT, x, y, 1, h);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_RIGHT, x + w - 1, y, 1, h);
}

void psrc_ui_renderBordered(int e, int x, int y, int w, int h) {
    psrc_ui_renderElem(e, x, y, w, h);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_TOP, x, y, w, 1);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_BTM, x, y + h - 1, w, 1);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_LEFT, x, y, 1, h);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_RIGHT, x + w - 1, y, 1, h);
}

void psrc_ui_renderDialogBase(int x, int y, int w, int h, bool tbar, char* title, bool logo, uint8_t btns) {
    (void)title;
    if (tbar) {
        psrc_ui_renderElem(PSRC_UI_WIN_TBAR, x, y, w, 24);
        psrc_ui_renderElem(PSRC_UI_WIN_TBAR_LEFT, x, y, 24, 24);
        psrc_ui_renderElem(PSRC_UI_WIN_TBAR_RIGHT, x + w - 24, y, 24, 24);
        y += 24;
    }
    psrc_ui_renderElem(PSRC_UI_WIN_FILL, x, y, w, h);
    if (!tbar) {
        psrc_ui_renderElem(PSRC_UI_WIN_BDR_TOP, x, y, w, 1);
    }
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_BTM, x, y + h - 1, w, 1);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_LEFT, x, y, 1, h);
    psrc_ui_renderElem(PSRC_UI_WIN_BDR_RIGHT, x + w - 1, y, 1, h);
    if (tbar) {
        if (logo) psrc_ui_renderElem(PSRC_UI_WIN_LOGO, x + 4, y - 20, 16, 16);
        if (btns & PSRC_UI_BTN_CLOSE) {psrc_ui_renderElem(PSRC_UI_WIN_CLOSE, x + w - 20, y - 20, 16, 16); psrc_ui_renderBorder(x + w - 20, y - 20, 16, 16);}
        if (btns & PSRC_UI_BTN_RESIZE) {psrc_ui_renderElem(PSRC_UI_WIN_RESIZE, x + w - 40, y - 20, 16, 16); psrc_ui_renderBorder(x + w - 40, y - 20, 16, 16);}
        if (btns & PSRC_UI_BTN_HELP) {psrc_ui_renderElem(PSRC_UI_WIN_HELP, x + w - 60, y - 20, 16, 16); psrc_ui_renderBorder(x + w - 60, y - 20, 16, 16);}
    }
}

void psrc_ui_renderDialog(psrc_ui_dialog* box) {
    psrc_ui_renderDialogBase(box->pos.x, box->pos.y, box->size.x, box->size.y, box->tbar, box->title, box->logo, box->btns);
    int x = box->pos.x, y = box->pos.y + 24 * box->tbar;
    for (int i = 0; i < box->elemct; ++i) {
        switch (box->elems[i].type) {
            case PSRC_UI_ELEM_BTN:
                psrc_ui_renderBordered(PSRC_UI_WIN_BTN, x + box->elems[i].pos.x, y + box->elems[i].pos.y, box->elems[i].size.x, box->elems[i].size.y);
                break;
            case PSRC_UI_ELEM_TBOX:
                //psrc_ui_renderBordered(PSRC_UI_WIN_BTN, elems[i].pos.x, elems[i].pos.y, elems[i].size.x, elems[i].size.y);
                break;
        }
    }
}

int psrc_ui_renderHookTestI = 0;
uint64_t psrc_ui_renderHookTime = 0;

psrc_ui_dialog* psrc_ui_dialogstack[256];
int psrc_ui_dialogstackp = 0;

void psrc_ui_pushToFront(uint16_t id) {
    int p = -1;
    for (int i = 0; i < psrc_ui_dialogstackp; ++i) {
        if (id == psrc_ui_dialogstack[i]->id) {p = i; break;}
    }
    if (p == -1) return;
    psrc_ui_dialog* box = psrc_ui_dialogstack[p];
    for (int i = p; i < psrc_ui_dialogstackp - 1; ++i) {
        psrc_ui_dialogstack[i] = psrc_ui_dialogstack[i + 1];
    }
    psrc_ui_dialogstack[psrc_ui_dialogstackp - 1] = box;
}

bool psrc_ui_curPressed = false;

void psrc_ui_pollUI() {
    double xpos, ypos;
    glfwGetCursorPos(psrc.gfx->window, &xpos, &ypos);
    static bool mposset = false;
    static double mxpos, mypos;
    if (!mposset) {
        mxpos = xpos;
        mypos = ypos;
        mposset = true;
    }
    double xdiff = xpos - mxpos;
    double ydiff = ypos - mypos;
    static bool act = false;
    static int clickid = -1;
    static bool clicktb = false;
    if (psrc_ui_curPressed && glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    } else {
        for (int i = psrc_ui_dialogstackp - 1; i > -1; --i) {
            //printf("clickid: 0: [%d]\n", clickid);
            psrc_ui_dialog* box = psrc_ui_dialogstack[i];
            if ((clickid == -1 || clickid == (int)box->id) && !act) {
                if (mxpos >= box->pos.x && mxpos < box->pos.x + box->size.x && mypos >= box->pos.y && mypos < box->pos.y + 24 * box->tbar + box->size.y) {
                    if (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && (clickid == -1 || clicktb)) {
                        if (clickid == -1) clicktb = true;
                        clickid = box->id;
                        if (i != psrc_ui_dialogstackp - 1) psrc_ui_pushToFront(box->id);
                        if (mypos < box->pos.y + 24 * box->tbar) {
                            box->pos.x += xdiff;
                            box->pos.y += ydiff;
                            if (box->pos.x < -box->size.x + 48) box->pos.x = -box->size.x + 48;
                            if (box->pos.y < 0) box->pos.y = 0;
                        } else {
                            clicktb = false;
                        }
                        act = true;
                    }
                } else if (i == 0) {
                    if (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) clickid = -2;
                }
            } else {
                act = false;
            }
            if (box->pos.x > (int)psrc.gfx->cur_width - 48) box->pos.x = psrc.gfx->cur_width - 48;
            if (box->pos.y > (int)psrc.gfx->cur_height - 24) box->pos.y = psrc.gfx->cur_height - 24;
            //printf("clickid: 1: [%d], clicktb: 1: [%d]\n", clickid, clicktb);
        }
        psrc_ui_curPressed = false;
    }
    if (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) clickid = -1;
    mxpos = xpos;
    mypos = ypos;
}

void psrc_ui_renderHook() {
    if (!psrc_ui.shown) return;
    for (int i = 0; i < psrc_ui_dialogstackp; ++i) {
        psrc_ui_renderDialog(psrc_ui_dialogstack[i]);
    }
}

void psrc_ui_showUI() {
    psrc_ui_curPressed = (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    psrc_ui.shown = true;
}

void psrc_ui_hideUI() {
    psrc_ui.shown = false;
}

uint16_t psrc_ui_newDialog(int x, int y, int w, int h, bool tbar, char* title, bool logo, uint8_t btns, int elemct, ...) {
    if (x < 0) x = psrc.gfx->cur_width / 2 - w / 2;
    if (y < 0) y = psrc.gfx->cur_height / 2 - h / 2;
    psrc_ui_dialog* box = malloc(sizeof(psrc_ui_dialog));
    box->pos = (psrc_ui_coord){x, y};
    box->size = (psrc_ui_coord){w, h};
    box->tbar = tbar;
    box->title = strdup(title);
    box->logo = logo;
    box->btns = btns;
    va_list elems;
    va_start(elems, elemct);
    box->elemct = elemct;
    box->elems = malloc(elemct * sizeof(psrc_ui_elem));
    for (int i = 0; i < elemct; ++i) {
        box->elems[i].type = va_arg(elems, int);
        box->elems[i].id = strdup(va_arg(elems, char*));
        box->elems[i].callback = va_arg(elems, void*);
        box->elems[i].pos.x = va_arg(elems, int);
        box->elems[i].pos.y = va_arg(elems, int);
        box->elems[i].size.x = va_arg(elems, int);
        box->elems[i].size.y = va_arg(elems, int);
        switch (box->elems[i].type) {
            case PSRC_UI_ELEM_BTN:
                box->elems[i].data = strdup(va_arg(elems, char*));
                break;
            case PSRC_UI_ELEM_TBOX:
                box->elems[i].data = strdup(va_arg(elems, char*));
                break;
        }
    }
    va_end(elems);
    uint16_t id = 0;
    for (int i = 0; i < psrc_ui_dialogstackp; ++i) {
        if (id == psrc_ui_dialogstack[i]->id) {++id; i = -1;}
    }
    box->id = id;
    psrc_ui_dialogstack[psrc_ui_dialogstackp++] = box;
    return id;
}

psrc_ui_struct* psrc_ui_init() {
    psrc_ui = (psrc_ui_struct){psrc_ui_renderHook, psrc_ui_pollUI, psrc_ui_newDialog, NULL, NULL, psrc_ui_pushToFront, NULL,
        psrc_ui_showUI, psrc_ui_hideUI, psrc_ui_deinit, false};
    if (FT_Init_FreeType(&psrc_ui_ftlib)) {
        psrc.displayError(PSRC_ERR, "FT_Init_FreeType", "Failed to initialize FreeType 2 library");
        return NULL;
    }
    if (FT_New_Face(psrc_ui_ftlib, "resources/base/fonts/tahoma.ttf", 0, &psrc_ui_ftface)) {
        psrc.displayError(PSRC_ERR, "FT_New_Face", "Failed to initialize FreeType 2 face");
        FT_Done_FreeType(psrc_ui_ftlib);
        return NULL;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Set_Pixel_Sizes(psrc_ui_ftface, 0, 18);
    for (unsigned char glyph = 0; glyph < 128; ++glyph) {
        if (FT_Load_Char(psrc_ui_ftface, glyph, FT_LOAD_RENDER)) {
            psrc.displayError(PSRC_WRN, "FT_Load_Char", psrc.getFText("Failed to load glyph %d", glyph));
            continue;
        }
    }
    GLint tmp[2] = {psrc.gfx->texNearFilter, psrc.gfx->texFarFilter};
    psrc.gfx->texNearFilter = GL_NEAREST;
    psrc.gfx->texFarFilter = GL_NEAREST;
    psrc_ui_loadElem(PSRC_UI_WIN_FILL, "fill.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_LOGO, "logo.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_CLOSE, "close.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_RESIZE, "resize.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_HELP, "help.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_TBAR_LEFT, "tbar_left.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_TBAR, "tbar.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_TBAR_RIGHT, "tbar_right.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_TOP, "bdr_top.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_BTM, "bdr_btm.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_LEFT, "bdr_left.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_RIGHT, "bdr_right.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BTN, "btn.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BTN_HOVER, "btn_hover.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_BTN_PRESS, "btn_press.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_PBAR, "pbar.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_SLD_LEFT, "sld_left.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_SLD, "sld.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_SLD_RIGHT, "sld_right.bmp");
    psrc_ui_loadElem(PSRC_UI_WIN_TBOX, "tbox.bmp");
    psrc.gfx->texNearFilter = tmp[0];
    psrc.gfx->texFarFilter = tmp[1];
    psrc_ui_newDialog(-1, -1, 400, 300, true, "test", true, 255, 1,
        PSRC_UI_ELEM_BTN, "button 1", NULL, 10, 10, 56, 24, "Test");
    psrc_ui_newDialog(-1, -1, 160, 120, true, "test2", true, 0, 0);
    return &psrc_ui;
}
