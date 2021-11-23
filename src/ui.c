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

psrc_gfx_obj* psrc_ui_elemobj[16];

void psrc_ui_renderObj2D(psrc_gfx_obj obj, unsigned x, unsigned y, unsigned w, unsigned h) {
    obj.pos = (psrc_coord_3d){(float)x * 2 / (float)psrc.gfx->cur_width - 1, 1 - (((float)y * 2 + (float)h * 2) / (float)psrc.gfx->cur_height), 0};
    obj.scale = (psrc_coord_3d){(float)w * 2 / (float)psrc.gfx->cur_width, (float)h * 2 / (float)psrc.gfx->cur_height, 1};
    psrc.gfx->renderObj(&obj);
}

void psrc_ui_renderElem(int id, unsigned x, unsigned y, unsigned w, unsigned h) {
    psrc_ui_renderObj2D(*psrc_ui_elemobj[id], x, y, w, h);
}

void psrc_ui_renderWindowBase(bool tbar, char* title, bool logo, bool close, unsigned x, unsigned y, unsigned w, unsigned h) {
    (void)title;
    if (tbar) {
        psrc_ui_renderElem(4, x, y, w, 24);
        psrc_ui_renderElem(3, x, y, 24, 24);
        psrc_ui_renderElem(5, x + w - 24, y, 24, 24);
        y += 24;
    }
    psrc_ui_renderElem(0, x, y, w, h);
    if (!tbar) {
        psrc_ui_renderElem(6, x, y, w, 1);
    }
    psrc_ui_renderElem(7, x, y + h - 1, w, 1);
    psrc_ui_renderElem(8, x, y, 1, h);
    psrc_ui_renderElem(9, x + w - 1, y, 1, h);
    if (tbar) {
        if (logo) psrc_ui_renderElem(1, x + 4, y - 20, 16, 16);
        if (close) psrc_ui_renderElem(2, x + w - 20, y - 20, 16, 16);
    }
}

int psrc_ui_renderHookTestI = 0;
uint64_t psrc_ui_renderHookTime = 0;

void psrc_ui_renderHook() {
    if (!psrc_ui.shown) return;
    static bool setHookTime = false;
    if (!setHookTime) {psrc_ui_renderHookTime = psrc.utime(); setHookTime = true;}
    switch (psrc_ui_renderHookTestI) {
        case 0:; psrc_ui_renderWindowBase(true, "test", true, true, 10, 10, 160, 160); break;
        case 1:; psrc_ui_renderWindowBase(true, "test", false, true, 10, 10, 160, 160); break;
        case 2:; psrc_ui_renderWindowBase(true, "test", true, false, 10, 10, 160, 160); break;
        case 3:; psrc_ui_renderWindowBase(true, "test", false, false, 10, 10, 160, 160); break;
        case 4:; psrc_ui_renderWindowBase(false, "test", true, true, 10, 10, 160, 160); break;
    }
    if (psrc.utime() - psrc_ui_renderHookTime >= 1000000) {
        psrc_ui_renderHookTestI = (psrc_ui_renderHookTestI + 1) % 5;
        psrc_ui_renderHookTime = psrc.utime();
    }
}

void psrc_ui_showUI() {
    psrc_ui.shown = true;
}

void psrc_ui_hideUI() {
    psrc_ui.shown = false;
}

psrc_ui_struct* psrc_ui_init() {
    psrc_ui = (psrc_ui_struct){psrc_ui_renderHook, NULL, NULL, NULL, NULL, NULL,
        psrc_ui_showUI, psrc_ui_hideUI, psrc_ui_deinit, false};
    /*
    if (FT_Init_FreeType(&psrc_ui_ftlib)) {
        psrc.displayError(PSRC_ERR, "FT_Init_FreeType", "Failed to initialize FreeType 2 library");
        return NULL;
    }
    if (FT_New_Face(psrc_ui_ftlib, "resources/base/fonts/bahnschrift.ttf", 0, &psrc_ui_ftface)) {
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
    */
    GLint tmp[2] = {psrc.gfx->texNearFilter, psrc.gfx->texFarFilter};
    psrc.gfx->texNearFilter = GL_NEAREST;
    psrc.gfx->texFarFilter = GL_NEAREST;
    psrc_ui_elemobj[0] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/fill.bmp", 0, 1, false);
    psrc_ui_elemobj[1] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/logo.bmp", 0, 1, false);
    psrc_ui_elemobj[2] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/close.bmp", 0, 1, false);
    psrc_ui_elemobj[3] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/tb_left.bmp", 0, 1, false);
    psrc_ui_elemobj[4] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/tb.bmp", 0, 1, false);
    psrc_ui_elemobj[5] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/tb_right.bmp", 0, 1, false);
    psrc_ui_elemobj[6] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/bdr_top.bmp", 0, 1, false);
    psrc_ui_elemobj[7] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/bdr_btm.bmp", 0, 1, false);
    psrc_ui_elemobj[8] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/bdr_left.bmp", 0, 1, false);
    psrc_ui_elemobj[9] = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_ui_testvertices, sizeof(psrc_ui_testvertices), psrc_ui_testindices, sizeof(psrc_ui_testindices),
        "resources/base/images/ui/bdr_right.bmp", 0, 1, false);
    psrc.gfx->texNearFilter = tmp[0];
    psrc.gfx->texFarFilter = tmp[1];
    return &psrc_ui;
}
