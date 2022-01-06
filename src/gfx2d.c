#include "psrc.h"
#include "gfx.h"
#include "gfx2d.h"
#include "main.h"

psrc_gfx2d_struct psrc_gfx2d;

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library psrc_gfx2d_ftlib;

float psrc_gfx2d_vertices[] = {
     0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
};

unsigned int psrc_gfx2d_indices[] = {
    0, 1, 2, 2, 3, 0
};

static inline void psrc_gfx2d_renderObj(psrc_gfx_obj* obj, int x, int y, int w, int h, float s) {
    psrc_coord_3d oldpos = obj->pos;
    psrc_coord_3d oldscale = obj->scale;
    obj->pos = (psrc_coord_3d){(float)x * 2 / (float)psrc.gfx->cur_width * s - 1, 1 - (((float)y * 2 + (float)h * 2) / (float)psrc.gfx->cur_height * s), 0};
    obj->scale = (psrc_coord_3d){(float)w * 2 / (float)psrc.gfx->cur_width * s, (float)h * 2 / (float)psrc.gfx->cur_height * s, 1};
    psrc.gfx->renderObj(obj);
    obj->pos = oldpos;
    obj->scale = oldscale;
}

static inline void psrc_gfx2d_renderText(psrc_gfx2d_font* font, char* str, float x, float y, float pt, psrc_color c) {
    if (!*str) return;
    glUniform1i(glGetUniformLocation(psrc.gfx->objsprog, "fIsText"), 1);
    glUniform3f(glGetUniformLocation(psrc.gfx->objsprog, "textColor"), c.r, c.g, c.b);
    float scale = pt / (float)font->pt;
    for (; *str; ++str) {
        psrc_gfx2d_chardata* chr = &font->charmap[(unsigned char)*str];
        float nx = x + (chr->bearing.x + font->spacing) * scale;
        float ny = y + (font->pt - chr->size.y + font->offset + chr->size.y - chr->bearing.y) * scale;
        psrc_gfx2d_renderObj(chr->obj, nx, ny, (float)chr->size.x * scale, (float)chr->size.y * scale, 1);
        x = nx + (float)(chr->advance >> 6) * scale;
    }
    glUniform1i(glGetUniformLocation(psrc.gfx->objsprog, "fIsText"), 0);
}

static inline int psrc_gfx2d_getTextWidth(psrc_gfx2d_font* font, char* str, int pt) {
    float x = 0.5;
    float scale = pt / (float)font->pt;
    for (; *str; ++str) {
        psrc_gfx2d_chardata* chr = &font->charmap[(unsigned char)*str];
        x += (chr->bearing.x + font->spacing + (float)(chr->advance >> 6)) * scale;
    }
    return x;
}

psrc_gfx_obj* psrc_gfx2d_new2DObj(char* t) {
    return psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){0, 0, 0},
        psrc_gfx2d_vertices, sizeof(psrc_gfx2d_vertices), psrc_gfx2d_indices, sizeof(psrc_gfx2d_indices),
        t, 0, 1, false);
}

psrc_gfx2d_font* psrc_gfx2d_loadFont(char* fontpath, int pt, int offset, int spacing) {
    FT_Face ftface;
    if (FT_New_Face(psrc_gfx2d_ftlib, fontpath, 0, &ftface)) {
        psrc.displayError(PSRC_ERR, "FT_New_Face", "Failed to initialize FreeType 2 face");
        FT_Done_FreeType(psrc_gfx2d_ftlib);
        return NULL;
    }
    psrc_gfx2d_font* font = malloc(sizeof(psrc_gfx2d_font));
    font->pt = pt;
    font->offset = offset;
    font->spacing = spacing;
    font->charmap = malloc(256 * sizeof(psrc_gfx2d_chardata));
    font->face = ftface;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Set_Pixel_Sizes(ftface, 0, pt);
    for (unsigned int chr = 0; chr < 256; ++chr) {
        if (FT_Load_Char(ftface, chr, FT_LOAD_RENDER)) {
            psrc.displayError(PSRC_WRN, "FT_Load_Char", psrc.getFText("Failed to load char %d", chr));
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            ftface->glyph->bitmap.width,
            ftface->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE,
            ftface->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        psrc_gfx_obj* obj = psrc_gfx2d_new2DObj(NULL);
        obj->texture = texture;
        font->charmap[chr].obj = obj;
        font->charmap[chr].size = (psrc_coord_2d){ftface->glyph->bitmap.width, ftface->glyph->bitmap.rows};
        font->charmap[chr].bearing = (psrc_coord_2d){ftface->glyph->bitmap_left, ftface->glyph->bitmap_top};
        font->charmap[chr].advance = ftface->glyph->advance.x;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
    return font;
}

void psrc_gfx2d_freeFont(psrc_gfx2d_font* font) {
    free(font->charmap);
    free(font);
}

void psrc_gfx2d_deinit() {
    FT_Done_FreeType(psrc_gfx2d_ftlib);
}

psrc_gfx2d_struct* psrc_gfx2d_init() {
    psrc_gfx2d = (psrc_gfx2d_struct){psrc_gfx2d_renderObj, psrc_gfx2d_loadFont, psrc_gfx2d_freeFont, psrc_gfx2d_renderText,
        psrc_gfx2d_getTextWidth, psrc_gfx2d_new2DObj, psrc_gfx2d_deinit};
    if (FT_Init_FreeType(&psrc_gfx2d_ftlib)) {
        psrc.displayError(PSRC_ERR, "FT_Init_FreeType", "Failed to initialize FreeType 2 library");
        return NULL;
    }
    return &psrc_gfx2d;
}
