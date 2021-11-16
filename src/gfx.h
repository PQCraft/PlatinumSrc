#ifndef PSRC_GFX

#define PSRC_GFX

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct {
    float x;
    float y;
    float z;
} psrc_coord_3d;

typedef struct {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float tx;
    float ty;
} psrc_point_3d;

typedef struct {
    psrc_coord_3d pos;
    psrc_coord_3d rot;
    psrc_coord_3d scale;
    float* vertices;
    long unsigned int trict;
    long unsigned int vsize;
    unsigned int* indices;
    long unsigned int isize;
    unsigned int VBO, VAO, EBO;
    unsigned int texture;
} psrc_gfx_obj;

typedef struct {
    int win_width;
    int win_height;
    int fps;
    GLFWwindow* window;
    psrc_coord_3d campos;
    psrc_coord_3d camrot;
    float camfov;
    float posmult;
    float rotmult;
    char* vshader;
    char* fshader;
    void (*deinit)(void);
    psrc_gfx_obj* (*newObj)(psrc_coord_3d, psrc_coord_3d, psrc_coord_3d, float*, long unsigned int, unsigned int*, long unsigned int, char*);
    void (*renderObj)(psrc_gfx_obj*);
    void (*updateScreen)(void);
    void (*updateCam)(void);
    void (*test)(void);
    bool (*winQuit)(void);
} psrc_gfx;

#define PSRC_GFX_DEFAULT_POS (psrc_coord_3d){0, 0, 0}
#define PSRC_GFX_DEFAULT_ROT (psrc_coord_3d){0, 0, 0}
#define PSRC_GFX_DEFAULT_SCALE (psrc_coord_3d){1, 1, 1}
#define PSRC_GFX_DEFAULT_MAT4 {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}

psrc_gfx* psrc_gfx_init();

#endif
