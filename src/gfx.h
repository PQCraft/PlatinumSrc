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
    long unsigned int vsize;
    bool autofreev;
    unsigned int* indices;
    long unsigned int isize;
    bool autofreei;
    unsigned int VBO, VAO, EBO;
    unsigned int texture;
    psrc_gfx_material material;
    const void* scene;
} psrc_gfx_obj;

typedef struct {
    psrc_gfx_obj* top;
    psrc_gfx_obj* bottom;
    psrc_gfx_obj* left;
    psrc_gfx_obj* front;
    psrc_gfx_obj* right;
    psrc_gfx_obj* back;
} psrc_gfx_skybox;

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
    unsigned int win_fps;
    unsigned int full_width;
    unsigned int full_height;
    unsigned int full_fps;
    unsigned int cur_width;
    unsigned int cur_height;
    unsigned int cur_fps;
    bool vsync;
    bool fullscr;
    GLFWmonitor* monitor;
    GLFWwindow* window;
    psrc_coord_3d campos;
    psrc_coord_3d camrot;
    float camfov;
    GLuint objsprog;
    GLuint lightsprog;
    GLenum texNearFilter;
    GLenum texFarFilter;
    void (*deinit)(void);
    void (*updateCam)(void);
    psrc_gfx_skybox* (*newSkybox)(char*, char*);
    void (*setSkybox)(psrc_gfx_skybox*);
    psrc_gfx_light* (*getLight)(int);
    psrc_gfx_light* (*getNextLight)(void);
    void (*updateLight)(int);
    void (*setMaxLight)(int);
    psrc_gfx_obj* (*newObj)(psrc_coord_3d, psrc_coord_3d, psrc_coord_3d,
        float*, long unsigned int, unsigned int*, long unsigned int,
        char*, float, float, bool);
    psrc_gfx_obj* (*loadObj)(char*, int, char*, float, float, bool);
    void (*pushObj)(psrc_gfx_obj*);
    psrc_gfx_obj (*popObj)(void);
    void (*render)(void);
    void (*update)(void);
    void (*clear)(void);
    void (*renderObj)(psrc_gfx_obj*);
    void (*set2D)(bool);
    bool (*changeShader)(GLuint*, char*, char*);
    int (*chkKey)(int);
    void (*setFullscreen)(bool);
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
