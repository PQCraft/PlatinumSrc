#ifndef PSRC_GFX2D

#define PSRC_GFX2D

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    int x;
    int y;
} psrc_coord_2d;

typedef struct {
    psrc_coord_2d size;
    psrc_coord_2d bearing;
    unsigned int advance;
    psrc_gfx_obj* obj;
} psrc_gfx2d_chardata;

typedef struct {
    int pt;
    int offset;
    int spacing;
    psrc_gfx2d_chardata* charmap;
    FT_Face face;
} psrc_gfx2d_font;

typedef struct {
    void (*renderObj)(psrc_gfx_obj*, int, int, int, int, float);
    psrc_gfx2d_font* (*loadFont)(char*, int, int, int);
    void (*freeFont)(psrc_gfx2d_font*);
    void (*renderText)(psrc_gfx2d_font*, char*, float, float, float, psrc_color);
    int (*getTextWidth)(psrc_gfx2d_font*, char*, int);
    psrc_gfx_obj* (*new2DObj)(char*);
    void (*deinit)(void);
} psrc_gfx2d_struct;

psrc_gfx2d_struct* psrc_gfx2d_init();

#endif
