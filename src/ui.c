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

void psrc_ui_loadElem(int elem, char* prefix, char* img, char* suffix) {
    psrc_ui_elemobj[elem] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        psrc.getFText("%s%s%s", prefix, img, suffix), 0, 0.5, false);
}

void psrc_ui_loadUI(char* prefix, char* suffix) {
    psrc_ui_loadElem(PSRC_UI_WIN_FILL, prefix, "fill", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_FILL, prefix, "fill", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_LOGO, prefix, "logo", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_CLOSE, prefix, "close", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_RESIZE, prefix, "resize", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_HELP, prefix, "help", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_TBAR_LEFT, prefix, "tbar_left", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_TBAR, prefix, "tbar", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_TBAR_RIGHT, prefix, "tbar_right", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_TOP, prefix, "bdr_top", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_BTM, prefix, "bdr_btm", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_LEFT, prefix, "bdr_left", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_RIGHT, prefix, "bdr_right", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BDR_SOLID, prefix, "bdr_solid", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BTN, prefix, "btn", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BTN_HOVER, prefix, "btn_hover", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_BTN_PRESS, prefix, "btn_press", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_PBAR, prefix, "pbar", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_SLD_LEFT, prefix, "sld_left", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_SLD, prefix, "sld", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_SLD_RIGHT, prefix, "sld_right", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_TBOX, prefix, "tbox", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_CBOX_TRUE, prefix, "cbox_true", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_CBOX_FALSE, prefix, "cbox_false", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_RBTN_TRUE, prefix, "rbtn_true", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_RBTN_FALSE, prefix, "rbtn_false", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_DRPDN, prefix, "drpdn", suffix);
    psrc_ui_loadElem(PSRC_UI_WIN_LIST, prefix, "list", suffix);
}

void psrc_ui_renderElem(int id, int x, int y, int w, int h) {
    psrc_ui_renderObj2D(*psrc_ui_elemobj[id], x, y, w, h);
}

void psrc_ui_renderBorder(int x, int y, int w, int h, int b) {
    switch (b) {
        case PSRC_UI_BDR_CONVEX:;
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_TOP, x, y, w, 1);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_BTM, x, y + h - 1, w, 1);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_LEFT, x, y, 1, h);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_RIGHT, x + w - 1, y, 1, h);
            break;
        case PSRC_UI_BDR_CONCAVE:;
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_BTM, x, y, w, 1);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_TOP, x, y + h - 1, w, 1);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_RIGHT, x, y, 1, h);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_LEFT, x + w - 1, y, 1, h);
            break;
        case PSRC_UI_BDR_SOLID:;
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_SOLID, x, y, w, 1);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_SOLID, x, y + h - 1, w, 1);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_SOLID, x, y, 1, h);
            psrc_ui_renderElem(PSRC_UI_WIN_BDR_SOLID, x + w - 1, y, 1, h);
            break;
    }
}

void psrc_ui_renderBordered(int e, int x, int y, int w, int h, int b) {
    psrc_ui_renderElem(e, x, y, w, h);
    psrc_ui_renderBorder(x, y, w, h, b);
}

void psrc_ui_renderDialogBase(int x, int y, int w, int h, bool tbar, char* title, bool logo, uint8_t btns, uint8_t cbtns) {
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
        if (btns & PSRC_UI_BTN_CLOSE) {x -= 20; psrc_ui_renderBordered(PSRC_UI_WIN_CLOSE, x + w, y - 20, 16, 16,
            (cbtns & PSRC_UI_BTN_CLOSE) ? PSRC_UI_BDR_CONCAVE : PSRC_UI_BDR_CONVEX);}
        if (btns & PSRC_UI_BTN_RESIZE) {x -= 20; psrc_ui_renderBordered(PSRC_UI_WIN_RESIZE, x + w, y - 20, 16, 16,
            (cbtns & PSRC_UI_BTN_RESIZE) ? PSRC_UI_BDR_CONCAVE : PSRC_UI_BDR_CONVEX);}
        if (btns & PSRC_UI_BTN_HELP) {x -= 20; psrc_ui_renderBordered(PSRC_UI_WIN_HELP, x + w, y - 20, 16, 16,
            (cbtns & PSRC_UI_BTN_HELP) ? PSRC_UI_BDR_CONCAVE : PSRC_UI_BDR_CONVEX);}
    }
}

void psrc_ui_renderDialog(psrc_ui_dialog* box) {
    psrc_ui_renderDialogBase(box->pos.x, box->pos.y, box->size.x, box->size.y, box->tbar, box->title, box->logo, box->btns, box->cbtns);
    int x = box->pos.x, y = box->pos.y + 24 * box->tbar;
    for (int i = 0; i < box->elemct; ++i) {
        psrc_ui_elem* elem = &box->elems[i];
        int expos = elem->pos.x, eypos = elem->pos.y;
        int exsize = elem->size.x, eysize = elem->size.y;
        if (expos < 0) expos = box->size.x - exsize + expos;
        if (eypos < 0) eypos = box->size.y - eysize + eypos;
        if (exsize < 0) exsize = box->size.x - expos + exsize;
        if (eysize < 0) eysize = box->size.y - eypos + eysize;
        int ex = x + expos, ey = y + eypos;
        switch (box->elems[i].type) {
            case PSRC_UI_ELEM_BTN:
                psrc_ui_renderBordered(PSRC_UI_WIN_BTN, ex, ey, exsize, eysize, elem->border);
                break;
            case PSRC_UI_ELEM_TBOX:
                psrc_ui_renderBordered(PSRC_UI_WIN_TBOX, ex, ey, exsize, eysize, elem->border);
                break;
            case PSRC_UI_ELEM_PBAR:
                psrc_ui_renderElem(PSRC_UI_WIN_PBAR, ex, ey, (float)exsize * (float)(int)(intptr_t)elem->data / 100.0f, eysize);
                psrc_ui_renderBorder(ex, ey, exsize, eysize, elem->border);
                break;
            case PSRC_UI_ELEM_SLIDER:
                psrc_ui_renderElem(PSRC_UI_WIN_SLD, ex, ey + eysize / 2 - 8, exsize, 16);
                psrc_ui_renderElem(PSRC_UI_WIN_SLD_LEFT, ex, ey + eysize / 2 - 8, 16, 16);
                psrc_ui_renderElem(PSRC_UI_WIN_SLD_RIGHT, ex + exsize - 16, ey + eysize / 2 - 8, 16, 16);
                psrc_ui_renderBordered(PSRC_UI_WIN_BTN, ex + (exsize - 16) * (float)(int)(intptr_t)elem->data / 100.0f, ey, 16, eysize, elem->border);
                break;
            case PSRC_UI_ELEM_CBOX:
                psrc_ui_renderBordered(((int)(intptr_t)elem->data) ? PSRC_UI_WIN_CBOX_TRUE : PSRC_UI_WIN_CBOX_FALSE,
                    ex + exsize / 2 - 8, ey + eysize / 2 - 8, 16, 16, elem->border);
                break;
            case PSRC_UI_ELEM_RBTN:
                psrc_ui_renderBordered(((int)(intptr_t)elem->data) ? PSRC_UI_WIN_RBTN_TRUE : PSRC_UI_WIN_RBTN_FALSE,
                    ex + exsize / 2 - 8, ey + eysize / 2 - 8, 16, 16, elem->border);
                break;
            case PSRC_UI_ELEM_LIST:
                psrc_ui_renderBordered(PSRC_UI_WIN_LIST, ex, ey, exsize, eysize, elem->border);
                break;
        }
    }
}

void psrc_ui_modDialog(uint16_t id, ...) {
    va_list args;
    va_start(args, id);
    //int act = va_arg(args, int);
    //int obj = va_arg(args, int);
    va_end(args);
}

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

void psrc_ui_clickDialog(uint16_t id, int mx, int my) {
    (void)id; (void)mx; (void)my;
    //printf("click on box %u at (%d, %d)\n", id, mx, my);
}

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
    static int clickid = -1;
    static bool clicktb = false;
    static bool clickhold = false;
    if (psrc_ui_curPressed && glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    } else {
        for (int i = psrc_ui_dialogstackp - 1; i > -1; --i) {
            //printf("clickid: 0: [%d]\n", clickid);
            psrc_ui_dialog* box = psrc_ui_dialogstack[i];
            if ((clickid == -1 || clickid == (int)box->id)) {
                if (mxpos >= box->pos.x && mxpos < box->pos.x + box->size.x && mypos >= box->pos.y && mypos < box->pos.y + 24 * box->tbar + box->size.y) {
                    if (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && (clickid == -1 || clicktb || clickhold)) {
                        if (clickid == -1) clicktb = true;
                        clickid = box->id;
                        if (i != psrc_ui_dialogstackp - 1) psrc_ui_pushToFront(box->id);
                        if (mypos < box->pos.y + 24 * box->tbar && clicktb) {
                            clickhold = false;
                            box->pos.x += xdiff;
                            box->pos.y += ydiff;
                            if (box->pos.x < -box->size.x + 48) box->pos.x = -box->size.x + 48;
                            if (box->pos.y < 0) box->pos.y = 0;
                        } else {
                            clicktb = false;
                            clickhold = true;
                            psrc_ui_clickDialog(box->id, mxpos - box->pos.x, mypos - box->pos.y - 24 * box->tbar);
                        }
                    }
                } else {
                    if (i == 0) {
                        if (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) clickid = -2;
                    }
                    clickhold = false;
                    clicktb = false;
                }
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

uint16_t psrc_ui_newDialog(int x, int y, int w, int h, bool tbar, char* title, bool logo, uint8_t btns, void* cb, int elemct, ...) {
    if (x < 0) x = psrc.gfx->cur_width / 2 - w / 2;
    if (y < 0) y = psrc.gfx->cur_height / 2 - h / 2;
    psrc_ui_dialog* box = malloc(sizeof(psrc_ui_dialog));
    memset(box, 0, sizeof(psrc_ui_dialog));
    box->pos = (psrc_ui_coord){x, y};
    box->size = (psrc_ui_coord){w, h};
    box->tbar = tbar;
    box->title = strdup(title);
    box->logo = logo;
    box->btns = btns;
    box->callback = cb;
    va_list elems;
    va_start(elems, elemct);
    if (elemct > 0) {
        box->elemct = elemct;
        box->elems = malloc(elemct * sizeof(psrc_ui_elem));
        memset(box->elems, 0, elemct * sizeof(psrc_ui_elem));
        for (int i = 0; i < elemct; ++i) {
            box->elems[i].type = va_arg(elems, int);
            box->elems[i].id = strdup(va_arg(elems, char*));
            box->elems[i].callback = va_arg(elems, void*);
            box->elems[i].pos.x = va_arg(elems, int);
            box->elems[i].pos.y = va_arg(elems, int);
            box->elems[i].size.x = va_arg(elems, int);
            box->elems[i].size.y = va_arg(elems, int);
            box->elems[i].border = va_arg(elems, int);
            switch (box->elems[i].type) {
                case PSRC_UI_ELEM_BTN:
                    box->elems[i].data = strdup(va_arg(elems, char*));
                    break;
                case PSRC_UI_ELEM_TBOX:
                    box->elems[i].data = strdup(va_arg(elems, char*));
                    break;
                case PSRC_UI_ELEM_PBAR:
                    box->elems[i].data = (void*)(intptr_t)va_arg(elems, int);
                    break;
                case PSRC_UI_ELEM_SLIDER:
                    box->elems[i].data = (void*)(intptr_t)va_arg(elems, int);
                    break;
                case PSRC_UI_ELEM_CBOX:
                    box->elems[i].data = (void*)(intptr_t)va_arg(elems, int);
                    break;
                case PSRC_UI_ELEM_RBTN:
                    box->elems[i].data = (void*)(intptr_t)va_arg(elems, int);
                    break;
                case PSRC_UI_ELEM_LIST:
                    int lelem = va_arg(elems, int);
                    char** src = va_arg(elems, char**);
                    char** dest = (box->elems[i].data = malloc(lelem * sizeof(char**)));
                    for (int i = 0; i < lelem; ++i) {
                        dest[i] = strdup(src[i]);
                    }
                    break;
            }
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
    psrc_ui = (psrc_ui_struct){psrc_ui_renderHook, psrc_ui_pollUI, psrc_ui_newDialog, NULL, NULL,
        psrc_ui_pushToFront, NULL, psrc_ui_loadUI, psrc_ui_showUI, psrc_ui_hideUI, psrc_ui_deinit, false};
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
    psrc_ui_loadUI("resources/base/images/ui/", ".bmp");
    psrc.gfx->texNearFilter = tmp[0];
    psrc.gfx->texFarFilter = tmp[1];
    psrc_ui_newDialog(-1, -1, 400, 300, true, "test", true, PSRC_UI_BTN_CLOSE | PSRC_UI_BTN_RESIZE | PSRC_UI_BTN_HELP, NULL, 9,
        PSRC_UI_ELEM_BTN, "button", NULL, 10, 10, 128, 24, PSRC_UI_BDR_CONVEX, "Test",
        PSRC_UI_ELEM_TBOX, "textbox", NULL, 10, 44, -10, 24, PSRC_UI_BDR_CONCAVE, "Test text",
        PSRC_UI_ELEM_PBAR, "progressbar", NULL, 10, 78, -10, 24, PSRC_UI_BDR_SOLID, 25,
        PSRC_UI_ELEM_SLIDER, "slider", NULL, 10, 112, -10, 24, PSRC_UI_BDR_CONVEX, 75,
        PSRC_UI_ELEM_CBOX, "checkbox 1", NULL, 10, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 0,
        PSRC_UI_ELEM_CBOX, "checkbox 2", NULL, 44, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 1,
        PSRC_UI_ELEM_RBTN, "radio 1", NULL, 78, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 0,
        PSRC_UI_ELEM_RBTN, "radio 2", NULL, 112, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 1,
        PSRC_UI_ELEM_LIST, "list", NULL, 10, 180, -10, -10, PSRC_UI_BDR_SOLID, 2, (char*[]){"test", "text"}
    );
    /*
    psrc_ui_newDialog(-1, -1, 160, 120, true, "test2", true, PSRC_UI_BTN_HELP, NULL, 3,
        PSRC_UI_ELEM_BTN, "b0", NULL, 10, 10, 56, 24, 1, "Test1",
        PSRC_UI_ELEM_BTN, "b1", NULL, -10, 44, 56, 24, 1, "Test2",
        PSRC_UI_ELEM_TBOX, "b2", NULL, 10, 78, 56, 24, 2, "Test3"
    );
    */
    return &psrc_ui;
}
