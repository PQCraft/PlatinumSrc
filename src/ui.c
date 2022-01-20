#include "psrc.h"
#include "ui.h"
#include "gfx.h"
#include "gfx2d.h"
#include "main.h"

psrc_ui_struct psrc_ui;

static inline void psrc_ui_deinit() {
}

float psrc_ui_elemvertices[] = {
     0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
     0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
};

unsigned int psrc_ui_elemindices[] = {
    0, 1, 2, 2, 3, 0
};

psrc_gfx_obj* psrc_ui_elemobj[256];

static inline void psrc_ui_loadElem(int elem, char* prefix, char* img, char* suffix) {
    psrc_ui_elemobj[elem] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_elemvertices, sizeof(psrc_ui_elemvertices), psrc_ui_elemindices, sizeof(psrc_ui_elemindices),
        psrc.getFText("%s%s%s", prefix, img, suffix), 0, 1, false);
}

static inline void psrc_ui_loadUI(char* prefix, char* suffix) {
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

static inline void psrc_ui_renderElem(int id, int x, int y, int w, int h) {
    psrc.gfx2d->renderObj(psrc_ui_elemobj[id], x, y, w, h, psrc_ui.scale);
}

static inline void psrc_ui_renderBorder(int x, int y, int w, int h, int b) {
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

static inline void psrc_ui_renderBordered(int e, int x, int y, int w, int h, int b) {
    psrc_ui_renderElem(e, x, y, w, h);
    psrc_ui_renderBorder(x, y, w, h, b);
}

void psrc_ui_renderText(char* text, int x, int y, int pt, psrc_color c) {
    psrc.gfx2d->renderText(psrc_ui.font, text, x * psrc.ui->scale, y * psrc.ui->scale, pt * psrc.ui->scale, c);
}

static inline void psrc_ui_renderDialogBase(int x, int y, int w, int h, bool tbar, char* title, bool logo, uint8_t btns, uint8_t cbtns) {
    if (tbar) {
        psrc_ui_renderElem(PSRC_UI_WIN_TBAR, x, y, w, 24);
        psrc_ui_renderElem(PSRC_UI_WIN_TBAR_LEFT, x, y, 24, 24);
        psrc_ui_renderElem(PSRC_UI_WIN_TBAR_RIGHT, x + w - 24, y, 24, 24);
        psrc_ui_renderText(title, x + 24, y + 5, 12, (psrc_color){0.9, 0.9, 0.9});
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

static inline void psrc_ui_renderDialog(psrc_ui_dialog* box) {
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
        int xs;
        switch (elem->type) {
            case PSRC_UI_ELEM_LABEL:
                psrc_ui_renderText(elem->data, box->pos.x + expos + 4, box->pos.y + 24 * box->tbar + eypos + eysize / 2 - 7, 12, (psrc_color){0.9, 0.9, 0.9});
                break;
            case PSRC_UI_ELEM_BTN:
                psrc_ui_renderBordered(PSRC_UI_WIN_BTN, ex, ey, exsize, eysize, elem->border);
                xs = psrc.gfx2d->getTextWidth(psrc_ui.font, elem->data, 12);
                psrc_ui_renderText(elem->data, box->pos.x + expos + exsize / 2 - xs / 2, box->pos.y + 24 * box->tbar + eypos + eysize / 2 - 7, 12, (psrc_color){0.9, 0.9, 0.9});
                break;
            case PSRC_UI_ELEM_TBOX:
                psrc_ui_renderBordered(PSRC_UI_WIN_TBOX, ex, ey, exsize, eysize, elem->border);
                if (!elem->outdata || !*(char*)elem->outdata) {
                    psrc_ui_renderText(elem->data, box->pos.x + expos + 4, box->pos.y + 24 * box->tbar + eypos + eysize / 2 - 7, 12, (psrc_color){0.65, 0.65, 0.65});
                }
                break;
            case PSRC_UI_ELEM_PBAR:
                psrc_ui_renderElem(PSRC_UI_WIN_PBAR, ex, ey, (float)exsize * (float)(int)(intptr_t)elem->data / 100.0f, eysize);
                psrc_ui_renderBorder(ex, ey, exsize, eysize, elem->border);
                char* pcnt = psrc.getFText("%d%%", (uintptr_t)elem->data);
                xs = psrc.gfx2d->getTextWidth(psrc_ui.font, pcnt, 12);
                psrc_ui_renderText(pcnt, box->pos.x + expos + exsize / 2 - xs / 2, box->pos.y + 24 * box->tbar + eypos + eysize / 2 - 7, 12, (psrc_color){0, 0, 0});
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

static inline void psrc_ui_modDialog(uint16_t id, ...) {
    va_list args;
    va_start(args, id);
    //int act = va_arg(args, int);
    //int obj = va_arg(args, int);
    va_end(args);
}

psrc_ui_dialog* psrc_ui_dialogstack[256];
int psrc_ui_dialogstackp = 0;

static inline psrc_ui_dialog* psrc_ui_getDialog(uint16_t id) {
    int p = -1;
    for (int i = 0; i < psrc_ui_dialogstackp; ++i) {
        if (id == psrc_ui_dialogstack[i]->id) {p = i; break;}
    }
    if (p == -1) return NULL;
    return psrc_ui_dialogstack[p];
}

static inline psrc_ui_elem* psrc_ui_getElem(psrc_ui_dialog* box, char* id) {
    for (int i = 0; i < box->elemct; ++i) {
        if (!strcmp(id, box->elems[i].id)) return &box->elems[i];
    }
    return NULL;
}

static inline void psrc_ui_pushToFront(uint16_t id) {
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

static inline void psrc_ui_dialogEvent(psrc_ui_dialog* box, psrc_ui_event event) {
    if (box->callback) box->callback(box, NULL, event);
}

static inline void psrc_ui_dialogClick(psrc_ui_dialog* box, bool md, int mx, int my) {
    static bool fc = true;
    static psrc_ui_elem* eelem = NULL;
    if (box->callback) {
        psrc_ui_event event;
        event.pos = (psrc_coord_2d){mx, my};
        if (md) {
            if (fc) {
                event.event = PSRC_UI_EVENT_CLICK;
                for (int i = box->elemct - 1; i > -1; --i) {
                    psrc_ui_elem* elem = &box->elems[i];
                    int expos = elem->pos.x, eypos = elem->pos.y;
                    int exsize = elem->size.x, eysize = elem->size.y;
                    if (expos < 0) expos = box->size.x - exsize + expos;
                    if (eypos < 0) eypos = box->size.y - eysize + eypos;
                    if (exsize < 0) exsize = box->size.x - expos + exsize;
                    if (eysize < 0) eysize = box->size.y - eypos + eysize;
                    if (mx >= expos && mx < expos + exsize && my >= eypos && my < eypos + eysize) {
                        eelem = elem; break;
                    }
                }
            } else {
                event.event = PSRC_UI_EVENT_HOLD;
            }
        } else {
            event.event = PSRC_UI_EVENT_RELEASE;
        }
        box->callback(box, eelem, event);
    }
    if ((fc = !md)) {
        eelem = NULL;
    }
}

static inline void psrc_ui_pollUI() {
    double mxpos, mypos;
    glfwGetCursorPos(psrc.gfx->window, &mxpos, &mypos);
    mxpos /= psrc_ui.scale; mypos /= psrc_ui.scale;
    mxpos = (int)mxpos; mypos = (int)mypos;
    static int clickid = -1;
    static psrc_ui_dialog* gbox = NULL;
    static bool clicktb = false;
    static bool clickhold = false;
    static int offx = 0, offy = 0;
    bool mousedown = (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    if (!psrc_ui_curPressed || !mousedown) {
        for (int i = psrc_ui_dialogstackp - 1; i > -1; --i) {
            //printf("click.0: [%d]: [%d][%d]\n", clickid, clicktb, clickhold);
            psrc_ui_dialog* box = psrc_ui_dialogstack[i];
            if ((clickid == -1 || clickid == (int)box->id)) {
                if ((mxpos >= box->pos.x && mxpos < box->pos.x + box->size.x && mypos >= box->pos.y && mypos < box->pos.y + 24 * box->tbar + box->size.y) || clicktb) {
                    if (mousedown && (clickid == -1 || clicktb || clickhold)) {
                        if (clickid == -1) {offx = box->pos.x - mxpos; offy = box->pos.y - mypos;}
                        clickid = box->id;
                        gbox = box;
                        if (i != psrc_ui_dialogstackp - 1) psrc_ui_pushToFront(box->id);
                        if (mypos < box->pos.y + 24 * box->tbar && !clickhold) {
                            clicktb = true;
                            clickhold = false;
                        }
                        if (clicktb) {
                            box->pos.x = mxpos + offx;
                            box->pos.y = mypos + offy;
                            if (box->pos.x < -box->size.x + 48) box->pos.x = -box->size.x + 48;
                            if (box->pos.y < 0) box->pos.y = 0;
                        } else {
                            if (!clicktb) clickhold = true;
                            psrc_ui_dialogClick(box, true, mxpos - box->pos.x, mypos - box->pos.y - 24 * box->tbar);
                        }
                    }
                } else {
                    if (i == 0 && !clickhold) {
                        if (mousedown) clickid = -2;
                    }
                    //clickhold = false;
                }
            }
            if (box->pos.x > (int)psrc.gfx->cur_width / psrc_ui.scale - 48) box->pos.x = psrc.gfx->cur_width / psrc_ui.scale - 48;
            if (box->pos.y > (int)psrc.gfx->cur_height / psrc_ui.scale - 24) box->pos.y = psrc.gfx->cur_height / psrc_ui.scale - 24;
            //printf("click.1: [%d]: [%d][%d]\n", clickid, clicktb, clickhold);
        }
        psrc_ui_curPressed = false;
    }
    if (!mousedown) {
        if (gbox) {
            if (clickid > -1 && !clicktb) {
                psrc_ui_dialogClick(gbox, false, mxpos - gbox->pos.x, mypos - gbox->pos.y - 24 * gbox->tbar);
            } else if (clicktb) {
                psrc_ui_dialogEvent(gbox, (psrc_ui_event){PSRC_UI_EVENT_MOVE, (psrc_coord_2d){gbox->pos.x, gbox->pos.y}});
            }
        }
        clickid = -1;
        clicktb = false;
        clickhold = false;
    }
}

psrc_gfx_obj* psrc_ui_bgobj;

static inline void psrc_ui_renderHook() {
    psrc.gfx2d->renderObj(psrc_ui_bgobj, 0, 0, psrc.gfx->cur_width, psrc.gfx->cur_height, 1);
    for (int i = 0; i < psrc_ui_dialogstackp; ++i) {
        psrc_ui_renderDialog(psrc_ui_dialogstack[i]);
    }
}

static inline void psrc_ui_showUI() {
    psrc_ui_curPressed = (glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    psrc_ui.shown = true;
}

static inline void psrc_ui_hideUI() {
    psrc_ui.shown = false;
}

psrc_coord_2d psrc_ui_startpos = {50, 50};

uint16_t psrc_ui_newDialog(int x, int y, int w, int h, bool tbar, char* title, bool logo, uint8_t btns, void* cb, int elemct, ...) {
    if (x < -1) x = psrc.gfx->cur_width / psrc_ui.scale / 2 - w / 2;
    if (y < -1) y = psrc.gfx->cur_height / psrc_ui.scale / 2 - h / 2;
    if (x == -1) {if (psrc_ui_startpos.x > (int)psrc.gfx->cur_width / psrc_ui.scale - w - 50) {psrc_ui_startpos.x = 50;} x = psrc_ui_startpos.x; psrc_ui_startpos.x += 50;}
    if (y == -1) {if (psrc_ui_startpos.y > (int)psrc.gfx->cur_height / psrc_ui.scale - h - 50) {psrc_ui_startpos.y = 50;} y = psrc_ui_startpos.y; psrc_ui_startpos.y += 50;}
    psrc_ui_dialog* box = malloc(sizeof(psrc_ui_dialog));
    memset(box, 0, sizeof(psrc_ui_dialog));
    box->pos = (psrc_coord_2d){x, y};
    box->size = (psrc_coord_2d){w, h};
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
            box->elems[i].pos.x = va_arg(elems, int);
            box->elems[i].pos.y = va_arg(elems, int);
            box->elems[i].size.x = va_arg(elems, int);
            box->elems[i].size.y = va_arg(elems, int);
            box->elems[i].border = va_arg(elems, int);
            switch (box->elems[i].type) {
                case PSRC_UI_ELEM_LABEL:
                    box->elems[i].data = strdup(va_arg(elems, char*));
                    break;
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
                    box->elems[i].data = va_arg(elems, void*);
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

void psrc_ui_closeDialog(uint16_t id) {
    int p = -1;
    for (int i = 0; i < psrc_ui_dialogstackp; ++i) {
        if (id == psrc_ui_dialogstack[i]->id) {p = i; break;}
    }
    if (p == -1) return;
    psrc_ui_dialog* box = psrc_ui_dialogstack[p];
    for (int i = 0; i < box->elemct; ++i) {
        psrc_ui_elem* elem = &box->elems[i];
        if (elem->type == PSRC_UI_ELEM_BTN || elem->type == PSRC_UI_ELEM_TBOX) {
            if (elem->data) free(elem->data);
        } else if (elem->type == PSRC_UI_ELEM_LIST) {
            psrc_ui_elem_list* listdata = elem->data;
            if (listdata->freelines) {
                for (int i = 0; i < listdata->linect; ++i) {
                    free(listdata->lines[i]);
                }
            }
            if (listdata->freearray) {
                free(listdata->lines);
            }
            if (listdata->freelist) {
                free(listdata);
            }
        }
    }
    psrc_ui_pushToFront(box->id);
    free(psrc_ui_dialogstack[psrc_ui_dialogstackp - 1]);
    --psrc_ui_dialogstackp;
}

psrc_ui_struct* psrc_ui_init() {
    psrc_ui = (psrc_ui_struct){psrc_ui_renderHook, psrc_ui_pollUI, psrc_ui_newDialog, NULL, psrc_ui_closeDialog,
        psrc_ui_pushToFront, NULL, psrc_ui_loadUI, psrc_ui_showUI, psrc_ui_hideUI, psrc_ui_deinit,
        psrc_ui_getDialog, psrc_ui_getElem,
        false, 1, psrc.gfx2d->loadFont("resources/base/fonts/ui.ttf", 12, -1, 0)};
    char* cfg = psrc.getTextFileSilent("config/base/ui.cfg");
    psrc_ui.scale = atof(psrc.getCfgVarStatic(cfg, "scale", "1"));
    free(cfg);
    GLint tmp[2] = {psrc.gfx->texNearFilter, psrc.gfx->texFarFilter};
    psrc.gfx->texNearFilter = GL_NEAREST;
    psrc.gfx->texFarFilter = GL_NEAREST;
    psrc_ui_loadUI("resources/base/images/ui/", ".bmp");
    psrc_ui_bgobj = psrc.gfx2d->new2DObj("resources/base/images/ui/bg.bmp");
    psrc.gfx->texNearFilter = tmp[0];
    psrc.gfx->texFarFilter = tmp[1];
    return &psrc_ui;
}
