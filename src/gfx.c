#include "psrc.h"
#include "gfx.h"

#include <math.h>
#include <cglm/cglm.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <sys/stat.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

psrc_gfx_struct psrc_gfx;
float psrc_gfx_aspect;

void psrc_gfx_setUniform1f(GLuint prog, char* name, float val) {
    int uHandle = glGetUniformLocation(prog, name);
    glUniform1f(uHandle, val);
}

void psrc_gfx_setUniform2f(GLuint prog, char* name, float val[2]) {
    int uHandle = glGetUniformLocation(prog, name);
    glUniform2f(uHandle, val[0], val[1]);
}

void psrc_gfx_setUniform3f(GLuint prog, char* name, float val[3]) {
    int uHandle = glGetUniformLocation(prog, name);
    glUniform3f(uHandle, val[0], val[1], val[2]);
}

void psrc_gfx_setUniform4f(GLuint prog, char* name, float val[4]) {
    int uHandle = glGetUniformLocation(prog, name);
    glUniform4f(uHandle, val[0], val[1], val[2], val[3]);
}

void psrc_gfx_setMat4(GLuint prog, char* name, mat4 val) {
    int uHandle = glGetUniformLocation(prog, name);
    glUniformMatrix4fv(uHandle, 1, GL_FALSE, *val);
}

void psrc_gfx_setUniform1i(GLuint prog, char* name, GLint val) {
    int uHandle = glGetUniformLocation(prog, name);
    glUniform1i(uHandle, val);
}

void psrc_gfx_updateCam() {
    mat4 view, projection;
    glm_perspective(psrc_gfx.camfov * M_PI / 180, psrc_gfx_aspect, 0.01f, 100.0f, projection);
    psrc_gfx_setMat4(psrc_gfx.objsprog, "projection", projection);
    vec3 direction = {cosf((psrc_gfx.camrot.y - 90) * M_PI / 180) * cosf(psrc_gfx.camrot.x * M_PI / 180),
        sin(psrc_gfx.camrot.x * M_PI / 180),
        sinf((psrc_gfx.camrot.y - 90) * M_PI / 180) * cosf(psrc_gfx.camrot.x * M_PI / 180)};
    direction[0] += psrc_gfx.campos.x;
    direction[1] += psrc_gfx.campos.y;
    direction[2] += psrc_gfx.campos.z;
    glm_lookat((vec3){psrc_gfx.campos.x, psrc_gfx.campos.y, psrc_gfx.campos.z},
        direction, (vec3){0, 1, 0}, view);
    psrc_gfx_setMat4(psrc_gfx.objsprog, "view", view);
    psrc_gfx_setUniform3f(psrc_gfx.objsprog, "viewPos", (float[]){psrc_gfx.campos.x, psrc_gfx.campos.y, psrc_gfx.campos.z});
}

void psrc_gfx_renderObj(psrc_gfx_obj* obj) {
    if (!obj) return;
    GLuint sprog = 0;
    switch (obj->type) {
        default:;
            sprog = psrc_gfx.objsprog;
            break;
        case PSRC_GFX_OBJ_LIGHT:;
            //sprog = psrc_gfx.lightsprog;
            psrc_gfx_setUniform3f(psrc_gfx.objsprog, "lightPos", (float[]){obj->pos.x, obj->pos.y, obj->pos.z});
            sprog = psrc_gfx.objsprog;
            break;
    }
    glUseProgram(sprog);
    psrc_gfx_setUniform1i(sprog, "objType", obj->type);
    psrc_gfx_setUniform3f(psrc_gfx.objsprog, "lightColor", (float[]){1, 1, 1});
    glBindVertexArray(obj->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
    glBufferData(GL_ARRAY_BUFFER, obj->vsize, obj->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj->isize, obj->indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    mat4 model = PSRC_GFX_DEFAULT_MAT4;
    glm_translate(model, (vec3){obj->pos.x, obj->pos.y, obj->pos.z});
    glm_rotate(model, obj->rot.x * M_PI / 180, (vec3){1, 0, 0});
    glm_rotate(model, obj->rot.y * M_PI / 180, (vec3){0, 1, 0});
    glm_rotate(model, obj->rot.z * M_PI / 180, (vec3){0, 0, 1});
    glm_scale(model, (vec3){obj->scale.x, obj->scale.y, obj->scale.z});
    psrc_gfx_setMat4(sprog, "model", model);
    if (obj->texture) {
        glBindTexture(GL_TEXTURE_2D, obj->texture);
        glUniform1i(glGetUniformLocation(sprog, "TexData"), 0);
        glUniform1i(glGetUniformLocation(sprog, "HasTex"), 1);
        glDrawElements(GL_TRIANGLES, obj->trict, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        glUniform1i(glGetUniformLocation(sprog, "HasTex"), 0);
        glDrawElements(GL_TRIANGLES, obj->trict, GL_UNSIGNED_INT, 0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void psrc_gfx_updateScreen() {
    glfwSwapBuffers(psrc_gfx.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool psrc_gfx_winQuit() {glfwPollEvents(); return glfwWindowShouldClose(psrc_gfx.window) || psrc.quitRequested;}

psrc_gfx_obj* psrc_gfx_newObj(int type, psrc_coord_3d p, psrc_coord_3d r, psrc_coord_3d s, float* v, long unsigned int vs, unsigned int* i, long unsigned int is, char* t) {
    int width, height, nrChannels;
    unsigned char* data;
    if (t) {
        data = stbi_load(t, &width, &height, &nrChannels, 0);
        if (!data) data = stbi_load("base/textures/base.bmp", &width, &height, &nrChannels, 0);
        if (!data) t = NULL;
    }
    psrc_gfx_obj* obj = malloc(sizeof(psrc_gfx_obj));
    memset(obj, 0, sizeof(psrc_gfx_obj));
    glGenVertexArrays(1, &obj->VAO);
    glGenBuffers(1, &obj->VBO);
    glGenBuffers(1, &obj->EBO);
    obj->type = type;
    obj->pos = p;
    obj->rot = r;
    obj->scale = s;
    obj->vertices = v;
    obj->vsize = vs;
    obj->indices = i;
    obj->isize = is;
    obj->trict = is / sizeof(*i);
    if (t) {
        glGenTextures(1, &obj->texture);
        glBindTexture(GL_TEXTURE_2D, obj->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    } else {
        obj->texture = 0;
    }
    return obj;
}

void psrc_gfx_deinit() {
    glfwTerminate();
}

void psrc_gfx_winch(GLFWwindow* win, int w, int h) {
    if (win == psrc_gfx.window) glViewport(0, 0, w, h);
}

int psrc_gfx_chkKey(int key) {
    return glfwGetKey(psrc_gfx.window, key);
}

bool psrc_gfx_makeShaderProg(char* vs, char* fs, GLuint* p) {
    char* vstext = psrc.getTextFile(vs);
    char* fstext = psrc.getTextFile(fs);
    if (!vstext || !fstext) return false;
    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexHandle, 1, (const GLchar * const*)&vstext, NULL);
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
        glDeleteShader(vertexHandle);
        goto retfalse;
    }
    GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragHandle, 1, (const GLchar * const*)&fstext, NULL);
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
        glDeleteShader(vertexHandle);
        glDeleteShader(fragHandle);
        goto retfalse;
    }
    *p = glCreateProgram();
    glAttachShader(*p, vertexHandle);
    glAttachShader(*p, fragHandle);
    glLinkProgram(*p);
    glGetProgramiv(*p, GL_LINK_STATUS, &ret);
    glDeleteShader(vertexHandle);
    glDeleteShader(fragHandle);
    if (!ret) {
        GLint logSize = 0;
        glGetProgramiv(*p, GL_INFO_LOG_LENGTH, &logSize);
        GLchar* log = malloc((logSize + 1) * sizeof(GLchar));
        glGetProgramInfoLog(*p, logSize, &logSize, log);
        log[logSize - 1] = 0;
        psrc.displayError(PSRC_ERR, "Shader program link error", (char*)log);
        free(log);
        goto retfalse;
    }
    free(vstext);
    free(fstext);
    return true;
    retfalse:;
    free(vstext);
    free(fstext);
    return false;
}

psrc_gfx_struct* psrc_gfx_init() {
    psrc_gfx = (psrc_gfx_struct){640, 480, 120, NULL, (psrc_coord_3d){0, 0, -4}, (psrc_coord_3d){0, 180, 0}, 50, 0, 0,
        psrc_gfx_deinit, psrc_gfx_newObj, psrc_gfx_renderObj, psrc_gfx_updateScreen, psrc_gfx_updateCam, psrc_gfx_chkKey, psrc_gfx_winQuit};
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if (!(psrc_gfx.window = glfwCreateWindow(psrc_gfx.win_width, psrc_gfx.win_height, PSRC_STR, NULL, NULL))) {
        psrc.displayError(PSRC_ERR, "glfwCreateWindow", "Failed to create window");
        return NULL;
    }
    glfwMakeContextCurrent(psrc_gfx.window);
    glfwSetInputMode(psrc_gfx.window, GLFW_STICKY_KEYS, GLFW_TRUE);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        psrc.displayError(PSRC_ERR, "gladLoadGLLoader", "Failed to initialize GLAD");
        return NULL;
    }
    glViewport(0, 0, psrc_gfx.win_width, psrc_gfx.win_height);
    glfwSetFramebufferSizeCallback(psrc_gfx.window, psrc_gfx_winch);
    if (!psrc_gfx_makeShaderProg("resources/base/shaders/vertex.glsl", "resources/base/shaders/fragment.glsl", &psrc_gfx.objsprog)) return NULL;
    //if (!psrc_gfx_makeShaderProg("resources/base/shaders/vertex_light.glsl", "resources/base/shaders/fragment_light.glsl", &psrc_gfx.lightsprog)) return NULL;
    psrc_gfx_aspect = (float)psrc_gfx.win_width / (float)psrc_gfx.win_height;
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glClearColor(0, 0, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(psrc_gfx.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(psrc_gfx.window);
    glfwPollEvents();
    return &psrc_gfx;
}
