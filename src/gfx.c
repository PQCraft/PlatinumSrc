#include "psrc.h"
#include "shaders.h"

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

psrc_gfx psrc_gfx_struct;

GLuint psrc_gfx_glprog;

const int FRAMETIME = 1000000 / 60;

psrc_coord_3d psrc_gfx_look = {0, 1, 0};

void psrc_gfx_drawCube(float ox, float oy, float oz) {
    glBegin(GL_QUADS);
    glNormal3f( 0.0 + ox,  0.0 + oy,  1.0 + oz);
    glVertex3f(-0.5 + ox, -0.5 + oy,  0.5 + oz);
    glVertex3f( 0.5 + ox, -0.5 + oy,  0.5 + oz);
    glVertex3f( 0.5 + ox,  0.5 + oy,  0.5 + oz);
    glVertex3f(-0.5 + ox,  0.5 + oy,  0.5 + oz);
    glNormal3f( 0.0 + ox,  0.0 + oy, -1.0 + oz);
    glVertex3f( 0.5 + ox, -0.5 + oy, -0.5 + oz);
    glVertex3f(-0.5 + ox, -0.5 + oy, -0.5 + oz);
    glVertex3f(-0.5 + ox,  0.5 + oy, -0.5 + oz);
    glVertex3f( 0.5 + ox,  0.5 + oy, -0.5 + oz);
    glNormal3f( 1.0 + ox,  0.0 + oy,  0.0 + oz);
    glVertex3f( 0.5 + ox, -0.5 + oy,  0.5 + oz);
    glVertex3f( 0.5 + ox, -0.5 + oy, -0.5 + oz);
    glVertex3f( 0.5 + ox,  0.5 + oy, -0.5 + oz);
    glVertex3f( 0.5 + ox,  0.5 + oy,  0.5 + oz);
    glNormal3f(-1.0 + ox,  0.0 + oy,  0.0 + oz);
    glVertex3f(-0.5 + ox, -0.5 + oy, -0.5 + oz);
    glVertex3f(-0.5 + ox, -0.5 + oy,  0.5 + oz);
    glVertex3f(-0.5 + ox,  0.5 + oy,  0.5 + oz);
    glVertex3f(-0.5 + ox,  0.5 + oy, -0.5 + oz);
    glNormal3f( 0.0 + ox,  1.0 + oy,  0.0 + oz);
    glVertex3f(-0.5 + ox,  0.5 + oy,  0.5 + oz);
    glVertex3f( 0.5 + ox,  0.5 + oy,  0.5 + oz);
    glVertex3f( 0.5 + ox,  0.5 + oy, -0.5 + oz);
    glVertex3f(-0.5 + ox,  0.5 + oy, -0.5 + oz);
    glNormal3f( 0.0 + ox, -1.0 + oy,  0.0 + oz);
    glVertex3f(-0.5 + ox, -0.5 + oy, -0.5 + oz);
    glVertex3f( 0.5 + ox, -0.5 + oy, -0.5 + oz);
    glVertex3f( 0.5 + ox, -0.5 + oy,  0.5 + oz);
    glVertex3f(-0.5 + ox, -0.5 + oy,  0.5 + oz);
    glEnd();
}

void psrc_gfx_render() {
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat[]){psrc_gfx_struct.campos.x, psrc_gfx_struct.campos.y, psrc_gfx_struct.campos.z, 1});
    //glRotatef(psrc_gfx_struct.camrot.x, 1, 0, 0);
    //glRotatef(psrc_gfx_struct.camrot.y, 0, 1, 0);
    //glTranslated(-psrc_gfx_struct.campos.x, -psrc_gfx_struct.campos.y, -psrc_gfx_struct.campos.z);
    //glPopMatrix();
    SDL_GL_SwapWindow(psrc_gfx_struct.window);
}

void psrc_gfx_deinit() {
    SDL_GL_DeleteContext(psrc_gfx_struct.context);
    SDL_DestroyWindow(psrc_gfx_struct.window);
}

int noargc = 0;
char** noargv = NULL;

psrc_gfx* psrc_gfx_init() {
    noargv = malloc(sizeof(char*));
    noargv[0] = malloc(1);
    noargv[0][0] = 0;
    psrc_gfx_struct = (psrc_gfx){640, 480, 0, 0, (psrc_coord_3d){0, 1, -4}, (psrc_coord_3d){0, 180, 0}, 0.1, 2, PSRC_GFX_VSHADER, PSRC_GFX_FSHADER, psrc_gfx_render, psrc_gfx_deinit};
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        psrc.displayError(PSRC_ERR, "SDL_Init", (char*)SDL_GetError());
        return NULL;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    if (!(psrc_gfx_struct.window = SDL_CreateWindow(PSRC_STR,
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   psrc_gfx_struct.win_width, psrc_gfx_struct.win_height,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN))) {
        psrc.displayError(PSRC_ERR, "SDL_CreateWindow", (char*)SDL_GetError());
        return NULL;
    }
    if (!(psrc_gfx_struct.context = SDL_GL_CreateContext(psrc_gfx_struct.window))) {
        psrc.displayError(PSRC_ERR, "SDL_GL_CreateContext", (char*)SDL_GetError());
        return NULL;
    }
    if (SDL_GL_SetSwapInterval(1) < 0) {
        psrc.displayError(PSRC_ERR, "SDL_GL_SetSwapInterval", (char*)SDL_GetError());
        return NULL;
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, psrc_gfx_struct.win_width - 1, psrc_gfx_struct.win_height - 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0, 0, 0.2, 0);
    glewInit();
    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexHandle, 1, &psrc_gfx_struct.vshader, NULL);
    glCompileShader(vertexHandle);
    GLint ret = GL_FALSE;
    glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        GLint logSize = 0;
        glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logSize);
        GLchar* log = malloc((logSize + 1) * sizeof(GLchar));
        glGetShaderInfoLog(vertexHandle, logSize, &logSize, log);
        log[logSize - 1] = 0;
        psrc.displayError(PSRC_ERR, "Vertex shader compile error", (char*)log);
        free(log);
    }
    GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragHandle, 1, &psrc_gfx_struct.fshader, NULL);
    glCompileShader(fragHandle);
    glGetShaderiv(fragHandle, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        GLint logSize = 0;
        glGetShaderiv(fragHandle, GL_INFO_LOG_LENGTH, &logSize);
        GLchar* log = malloc((logSize + 1) * sizeof(GLchar));
        glGetShaderInfoLog(fragHandle, logSize, &logSize, log);
        log[logSize - 1] = 0;
        psrc.displayError(PSRC_ERR, "Fragment shader compile error", (char*)log);
        free(log);
    }
    psrc_gfx_glprog = glCreateProgram();
    glAttachShader(psrc_gfx_glprog, vertexHandle);
    glAttachShader(psrc_gfx_glprog, fragHandle);
    glLinkProgram(psrc_gfx_glprog);
    glGetProgramiv(psrc_gfx_glprog, GL_LINK_STATUS, &ret);
    if (!ret) {
        GLint logSize = 0;
        glGetProgramiv(psrc_gfx_glprog, GL_INFO_LOG_LENGTH, &logSize);
        GLchar* log = malloc((logSize + 1) * sizeof(GLchar));
        glGetProgramInfoLog(psrc_gfx_glprog, logSize, &logSize, log);
        log[logSize - 1] = 0;
        psrc.displayError(PSRC_ERR, "Shader program link error", (char*)log);
        free(log);
    }
    glDeleteShader(vertexHandle);
    glDeleteShader(fragHandle);
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bool cont = false;
    const uint8_t* keyStates = SDL_GetKeyboardState(NULL);
    //glUseProgram(psrc_gfx_glprog);
    //int vertexColorLocation = glGetUniformLocation(psrc_gfx_glprog, "ourColor");
    while (!cont) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                cont = true;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                keyStates = SDL_GetKeyboardState(NULL);
            }
        }
        if (keyStates[SDL_SCANCODE_W]) {
            float yrotrad = (psrc_gfx_struct.camrot.y / 180 * M_PI);
            psrc_gfx_struct.campos.x += sinf(yrotrad) * psrc_gfx_struct.posmult;
            psrc_gfx_struct.campos.z -= cosf(yrotrad) * psrc_gfx_struct.posmult;
        }
        if (keyStates[SDL_SCANCODE_S]) {
            float yrotrad = (psrc_gfx_struct.camrot.y / 180 * M_PI);
            psrc_gfx_struct.campos.x -= sinf(yrotrad) * psrc_gfx_struct.posmult;
            psrc_gfx_struct.campos.z += cosf(yrotrad) * psrc_gfx_struct.posmult;
        }
        if (keyStates[SDL_SCANCODE_A]) {
            float yrotrad;
            yrotrad = (psrc_gfx_struct.camrot.y / 180 * M_PI);
            psrc_gfx_struct.campos.x -= cosf(yrotrad) * psrc_gfx_struct.posmult;
            psrc_gfx_struct.campos.z -= sinf(yrotrad) * psrc_gfx_struct.posmult;
        }
        if (keyStates[SDL_SCANCODE_D]) {
            float yrotrad;
            yrotrad = (psrc_gfx_struct.camrot.y / 180 * M_PI);
            psrc_gfx_struct.campos.x += cosf(yrotrad) * psrc_gfx_struct.posmult;
            psrc_gfx_struct.campos.z += sinf(yrotrad) * psrc_gfx_struct.posmult;
        }
        if (keyStates[SDL_SCANCODE_UP]) {
            if (psrc_gfx_struct.camrot.x > -90) psrc_gfx_struct.camrot.x -= psrc_gfx_struct.rotmult;
        }
        if (keyStates[SDL_SCANCODE_DOWN]) {
            if (psrc_gfx_struct.camrot.x < 90) psrc_gfx_struct.camrot.x += psrc_gfx_struct.rotmult;
        }
        if (keyStates[SDL_SCANCODE_LEFT]) {
            psrc_gfx_struct.camrot.y -= psrc_gfx_struct.rotmult;
            if (psrc_gfx_struct.camrot.y < -360) psrc_gfx_struct.camrot.y += 360;
        }
        if (keyStates[SDL_SCANCODE_RIGHT]) {
            psrc_gfx_struct.camrot.y += psrc_gfx_struct.rotmult;
            if (psrc_gfx_struct.camrot.y > 360) psrc_gfx_struct.camrot.y -= 360;
        }
        if (keyStates[SDL_SCANCODE_LSHIFT] || keyStates[SDL_SCANCODE_RSHIFT]) {
            psrc_gfx_struct.campos.y = 0.5;
        } else {
            psrc_gfx_struct.campos.y = 1;
        }
        //float timeValue = psrc.utime();
        //float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        //glUniform4f(vertexColorLocation, 0.0f, psrc.randfloat(0, 1), 0.0f, 1.0f);
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(psrc_gfx_glprog);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        SDL_GL_SwapWindow(psrc_gfx_struct.window);
        //psrc_gfx_struct.render();
    }
    return &psrc_gfx_struct;
}
