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

psrc_gfx psrc_gfx_struct;

GLuint psrc_gfx_glprog;

void psrc_gfx_setUniform1f(char* name, float val) {
    int uHandle = glGetUniformLocation(psrc_gfx_glprog, name);
    glUniform1f(uHandle, val);
}

void psrc_gfx_setUniform2f(char* name, float val[2]) {
    int uHandle = glGetUniformLocation(psrc_gfx_glprog, name);
    glUniform2f(uHandle, val[0], val[1]);
}

void psrc_gfx_setUniform3f(char* name, float val[3]) {
    int uHandle = glGetUniformLocation(psrc_gfx_glprog, name);
    glUniform3f(uHandle, val[0], val[1], val[2]);
}

void psrc_gfx_setUniform4f(char* name, float val[4]) {
    int uHandle = glGetUniformLocation(psrc_gfx_glprog, name);
    glUniform4f(uHandle, val[0], val[1], val[2], val[3]);
}

void psrc_gfx_render(psrc_gfx_obj* obj) {
    glBindVertexArray(obj->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
    glBufferData(GL_ARRAY_BUFFER, obj->vsize, obj->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj->isize, obj->indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(glGetUniformLocation(psrc_gfx_glprog, "texture1"), 0);
    glUseProgram(psrc_gfx_glprog);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glfwSwapBuffers(psrc_gfx_struct.window);
    glfwPollEvents();
}

void psrc_gfx_updateScreen() {
    glfwSwapBuffers(psrc_gfx_struct.window);
}

bool psrc_gfx_winQuit() {glfwPollEvents(); return glfwWindowShouldClose(psrc_gfx_struct.window) || psrc.quitRequested;}

psrc_gfx_obj* psrc_gfx_newObj(psrc_coord_3d p, psrc_coord_3d r, float* v, long unsigned int vs, unsigned int* i, long unsigned int is, char* t) {
    struct stat pathstat;
    if (stat(t, &pathstat)) return NULL;
    psrc_gfx_obj* obj = malloc(sizeof(psrc_gfx_obj));
    memset(obj, 0, sizeof(psrc_gfx_obj));
    glGenVertexArrays(1, &obj->VAO);
    glGenBuffers(1, &obj->VBO);
    glGenBuffers(1, &obj->EBO);
    glGenTextures(1, &obj->texture);
    obj->pos = p;
    obj->rot = r;
    obj->vertices = v;
    obj->vsize = vs;
    obj->indices = i;
    obj->isize = is;
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(t, &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return obj;
}

void psrc_gfx_deinit() {
    glfwTerminate();
    free(psrc_gfx_struct.vshader);
    free(psrc_gfx_struct.fshader);
}

void psrc_gfx_winch(GLFWwindow* win, int w, int h) {
    if (win == psrc_gfx_struct.window) glViewport(0, 0, w, h);
}

psrc_gfx* psrc_gfx_init() {
    psrc_gfx_struct = (psrc_gfx){640, 480, 60, 0, (psrc_coord_3d){0, 1, -4}, (psrc_coord_3d){0, 180, 0}, 0.1, 2,
        psrc.getTextFile("common/shaders/vertex.glsl"), psrc.getTextFile("common/shaders/fragment.glsl"),
        psrc_gfx_render, psrc_gfx_deinit, psrc_gfx_newObj, psrc_gfx_updateScreen, psrc_gfx_winQuit};
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if (!(psrc_gfx_struct.window = glfwCreateWindow(psrc_gfx_struct.win_width, psrc_gfx_struct.win_height, PSRC_STR, NULL, NULL))) {
        psrc.displayError(PSRC_ERR, "glfwCreateWindow", "Failed to create window");
        return NULL;
    }
    glfwMakeContextCurrent(psrc_gfx_struct.window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        psrc.displayError(PSRC_ERR, "gladLoadGLLoader", "Failed to initialize GLAD");
        return NULL;
    }
    glViewport(0, 0, psrc_gfx_struct.win_width, psrc_gfx_struct.win_height);
    glfwSetFramebufferSizeCallback(psrc_gfx_struct.window, psrc_gfx_winch);
    #if 1
    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexHandle, 1, (const GLchar * const*)&psrc_gfx_struct.vshader, NULL);
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
    glShaderSource(fragHandle, 1, (const GLchar * const*)&psrc_gfx_struct.fshader, NULL);
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
    glUseProgram(psrc_gfx_glprog);
    #endif
    stbi_set_flip_vertically_on_load(true);
    glClearColor(0, 0, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(psrc_gfx_struct.window);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(psrc_gfx_struct.window);
    glfwPollEvents();
    return &psrc_gfx_struct;
}
