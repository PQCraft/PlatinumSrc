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
    float r;
    float g;
    float b;
} psrc_color;

typedef struct {
    unsigned int diffuse;
    unsigned int specular;
    float shine;
    float lightResistance;
} psrc_gfx_material;

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
    psrc_gfx_material material;
} psrc_gfx_obj;

typedef struct {
    int id;
    int type;
    psrc_coord_3d pos;
    psrc_color ambient;
    psrc_color diffuse;
    psrc_color specular;       
    psrc_coord_3d direction;
    float range;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    psrc_coord_3d topCorner;
    psrc_coord_3d bottomCorner;
} psrc_gfx_light;

typedef struct {
    unsigned int win_width;
    unsigned int win_height;
    unsigned int fps;
    bool vsync;
    GLFWmonitor* monitor;
    GLFWwindow* window;
    psrc_coord_3d campos;
    psrc_coord_3d camrot;
    float camfov;
    GLuint objsprog;
    GLuint lightsprog;
    void (*deinit)(void);
    psrc_gfx_obj* (*newObj)(psrc_coord_3d, psrc_coord_3d, psrc_coord_3d,
        float*, long unsigned int, unsigned int*, long unsigned int,
        char*, float, float);
    void (*renderObj)(psrc_gfx_obj*);
    psrc_gfx_light* (*getLight)(int);
    psrc_gfx_light* (*getNextLight)(void);
    void (*updateLight)(int);
    void (*updateScreen)(void);
    void (*updateCam)(void);
    int (*chkKey)(int);
    bool (*winQuit)(void);
} psrc_gfx_struct;

#define PSRC_GFX_DEFAULT_POS (psrc_coord_3d){0, 0, 0}
#define PSRC_GFX_DEFAULT_ROT (psrc_coord_3d){0, 0, 0}
#define PSRC_GFX_DEFAULT_SCALE (psrc_coord_3d){1, 1, 1}
#define PSRC_GFX_DEFAULT_MAT4 {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}

#define PSRC_GFX_LIGHT_DISABLED (0)
#define PSRC_GFX_LIGHT_DEFAULT (1)
#define PSRC_GFX_LIGHT_DIRECTION (2)
#define PSRC_GFX_LIGHT_POINT (3)
#define PSRC_GFX_LIGHT_SPOT (4)

psrc_gfx_struct* psrc_gfx_init();

#endif
