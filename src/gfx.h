#ifndef PSRC_GFX

#define PSRC_GFX

typedef struct {
    float x;
    float y;
    float z;
} psrc_coord_3d;

typedef struct {
    int win_width;
    int win_height;
    SDL_Window* window;
    SDL_GLContext context;
    psrc_coord_3d campos;
    psrc_coord_3d camrot;
    float posmult;
    float rotmult;
    const char* vshader;
    const char* fshader;
    void (*render)(void);
    void (*deinit)(void);
} psrc_gfx;

psrc_gfx* psrc_gfx_init();

#endif
