#include "psrc.h"

#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

#ifndef MAX_PATH
#define MAX_PATH 32768
#endif

psrc_main_struct psrc;

uint64_t psrc_main_utime() {
    struct timeval time1;
    gettimeofday(&time1, NULL);
    return time1.tv_sec * 1000000 + time1.tv_usec;
}

void psrc_main_wait(uint64_t d) {
    #ifndef _WIN32
    struct timespec dts;
    dts.tv_sec = d / 1000000;
    dts.tv_nsec = (d % 1000000) * 1000;
    nanosleep(&dts, NULL);
    #else
    uint64_t t = d + psrc_main_utime();
    while (t > psrc_main_utime()) {}
    #endif
}

float psrc_main_randfloat(float num1, float num2) {
    float range = num2 - num1;
    float div = RAND_MAX / range;
    return num1 + (rand() / div);
}

char* psrc_main_getErrorText_text = NULL;

char* psrc_main_getErrorText(char* func, char* estr) {
    psrc_main_getErrorText_text = realloc(psrc_main_getErrorText_text, strlen(func) + strlen(estr) + 3);
    sprintf(psrc_main_getErrorText_text, "%s: %s", func, estr);
    return psrc_main_getErrorText_text;
}

void psrc_main_displayError(int ecode, char* func, char* estr) {
    switch (ecode) {
        case PSRC_INF:;
            #ifndef _WIN32
            printf("i: %s\n", psrc_main_getErrorText(func, estr));
            #else
            MessageBox(NULL, psrc_main_getErrorText(func, estr), "Info", MB_OK | MB_ICONINFORMATION);
            #endif
            break;
        case PSRC_WRN:;
            #ifndef _WIN32
            printf("\e[33;1m!: %s\e[0m\n", psrc_main_getErrorText(func, estr));
            #else
            MessageBox(NULL, psrc_main_getErrorText(func, estr), "Warning", MB_OK | MB_ICONWARNING);
            #endif
            break;
        default:;
            #ifndef _WIN32
            printf("\e[31;1mX: %s\e[0m\n", psrc_main_getErrorText(func, estr));
            #else
            MessageBox(NULL, psrc_main_getErrorText(func, estr), "Error", MB_OK | MB_ICONERROR);
            #endif
            break;
    }
}

char* psrc_main_getTextFile(char* name) {
    struct stat fnst;
    memset(&fnst, 0, sizeof(struct stat));
    stat(name, &fnst);
    if (!S_ISREG(fnst.st_mode)) {
        psrc_main_displayError(PSRC_ERR, "psrc_main_getTextFile: Not a file", name);
        return NULL;
    }
    FILE* file = fopen(name, "r");
    if (!file) {
        psrc_main_displayError(PSRC_ERR, "psrc_main_getTextFile: File not found", name);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    char* data = malloc(size + 1);
    fseek(file, 0, SEEK_SET);
    long int i = 0;
    while (i < size && !feof(file)) {
        int tmpc = fgetc(file);
        if (tmpc < 0) tmpc = 0;
        data[i++] = (char)tmpc;
    }
    data[i] = 0;
    return data;
}

char* psrc_main_bfnbuf = NULL;

char* psrc_main_basefilename(char* fn) {
    int32_t fnlen = strlen(fn);
    int32_t i;
    for (i = fnlen; i > -1; --i) {
        if (fn[i] == '/') break;
        #ifdef _WIN32
        if (fn[i] == '\\') break;
        #endif
    }
    psrc_main_bfnbuf = realloc(psrc_main_bfnbuf, fnlen - i);
    strcpy(psrc_main_bfnbuf, fn + i + 1);
    return psrc_main_bfnbuf;
}

char* psrc_main_pathfilename(char* fn) {
    int32_t fnlen = strlen(fn);
    int32_t i;
    for (i = fnlen; i > -1; --i) {
        if (fn[i] == '/') break;
        #ifdef _WIN32
        if (fn[i] == '\\') break;
        #endif
    }
    psrc_main_bfnbuf = realloc(psrc_main_bfnbuf, i + 2);
    char tmp = fn[i + 1];
    fn[i + 1] = 0;
    strcpy(psrc_main_bfnbuf, fn);
    fn[i + 1] = tmp;
    return psrc_main_bfnbuf;
}

void psrc_main_cleanExit(int code) {
    free(psrc_main_getErrorText_text);
    if (psrc.sound && psrc.sound->deinit) psrc.sound->deinit();
    if (psrc.gfx && psrc.gfx->deinit) psrc.gfx->deinit();
    SDL_Quit();
    exit(code);
}

void psrc_main_cleanExitSig(int sig) {
    (void)sig;
    psrc.quitRequested = true;
}

float psrc_main_test_posmult = 0.25;
float psrc_main_test_rotmult = 3;

void psrc_main_test() {
    float pmult = psrc_main_test_posmult;
    float rmult = psrc_main_test_rotmult;
    if (psrc.gfx->chkKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || psrc.gfx->chkKey(GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        psrc.gfx->campos.y = 0.75;
        pmult /= 2;
    } else {
        psrc.gfx->campos.y = 1.5;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_LEFT_ALT) == GLFW_PRESS || psrc.gfx->chkKey(GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        psrc.gfx->camfov = 10;
        pmult /= 2;
        rmult /= 3;
    } else {
        psrc.gfx->camfov = 50;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_W) == GLFW_PRESS) {
        float yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
        psrc.gfx->campos.x += sinf(yrotrad) * pmult;
        psrc.gfx->campos.z -= cosf(yrotrad) * pmult;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_S) == GLFW_PRESS) {
        float yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
        psrc.gfx->campos.x -= sinf(yrotrad) * pmult;
        psrc.gfx->campos.z += cosf(yrotrad) * pmult;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_A) == GLFW_PRESS) {
        float yrotrad;
        yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
        psrc.gfx->campos.x -= cosf(yrotrad) * pmult;
        psrc.gfx->campos.z -= sinf(yrotrad) * pmult;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_D) == GLFW_PRESS) {
        float yrotrad;
        yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
        psrc.gfx->campos.x += cosf(yrotrad) * pmult;
        psrc.gfx->campos.z += sinf(yrotrad) * pmult;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_UP) == GLFW_PRESS) {
        if (psrc.gfx->camrot.x < 90 - rmult) psrc.gfx->camrot.x += rmult;
        else {psrc.gfx->camrot.x = 89.999;}
    }
    if (psrc.gfx->chkKey(GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (psrc.gfx->camrot.x > -90 + rmult) psrc.gfx->camrot.x -= rmult;
        else {psrc.gfx->camrot.x = -89.999;}
    }
    if (psrc.gfx->chkKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
        psrc.gfx->camrot.y -= rmult;
        if (psrc.gfx->camrot.y < -360) psrc.gfx->camrot.y += 360;
    }
    if (psrc.gfx->chkKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
        psrc.gfx->camrot.y += rmult;
        if (psrc.gfx->camrot.y > 360) psrc.gfx->camrot.y -= 360;
    }
    psrc.gfx->updateCam();
}

void psrc_main_test_renderObjAtFloors(psrc_gfx_obj* obj) {
    psrc.gfx->renderObj(obj);
    obj->pos.x += 40;
    psrc.gfx->renderObj(obj);
    obj->pos.x -= 40;
    obj->pos.z += 40;
    psrc.gfx->renderObj(obj);
    obj->pos.x += 40;
    psrc.gfx->renderObj(obj);
    obj->pos.x -= 40;
    obj->pos.z -= 40;
}

char psrc_main_getFTextBuf[32768];

char* psrc_main_getFText(char* f, ...) {
    va_list a;
    va_start(a, f);
    vsprintf(psrc_main_getFTextBuf, f, a);
    va_end(a);
    return psrc_main_getFTextBuf;
}

char* psrc_startcmd = NULL;

void psrc_main_init() {
    signal(SIGINT, psrc_main_cleanExitSig);
    psrc = (psrc_main_struct){psrc_main_displayError, psrc_main_wait, psrc_main_utime, psrc_main_randfloat, psrc_main_getTextFile, psrc_main_getFText, NULL, NULL, false};
    if (!(psrc.sound = psrc_sound_init())) psrc_main_cleanExit(1);
    if (!(psrc.gfx = psrc_gfx_init())) psrc_main_cleanExit(1);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    psrc_startcmd = malloc(MAX_PATH + 1);
    #ifndef _WIN32
        #ifndef __APPLE__
            #ifndef __FreeBSD__
                int32_t scsize;
                if ((scsize = readlink("/proc/self/exe", psrc_startcmd, MAX_PATH)) == -1)
                    #ifndef __linux__
                    if ((scsize = readlink("/proc/curproc/file", psrc_startcmd, MAX_PATH)) == -1)
                    #endif
                        goto scargv;
                psrc_startcmd[scsize] = 0;
                psrc_startcmd = realloc(psrc_startcmd, scsize + 1);
            #else
                int mib[4];
                mib[0] = CTL_KERN;
                mib[1] = KERN_PROC;
                mib[2] = KERN_PROC_PATHNAME;
                mib[3] = -1;
                size_t cb = MAX_PATH;
                sysctl(mib, 4, psrc_startcmd, &cb, NULL, 0);
                psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
                char* tmpstartcmd = realpath(psrc_startcmd, NULL);
                swap(psrc_startcmd, tmpstartcmd);
                nfree(tmpstartcmd);
                psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
            #endif
        #else
            uint32_t tmpsize = MAX_PATH;
            if (_NSGetExecutablePath(psrc_startcmd, &tmpsize)) {
                goto scargv;
            }
            char* tmpstartcmd = realpath(psrc_startcmd, NULL);
            swap(psrc_startcmd, tmpstartcmd);
            nfree(tmpstartcmd);
            psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
        #endif
    #else
        if (GetModuleFileName(NULL, psrc_startcmd, MAX_PATH)) {
            psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
        } else {
            goto scargv;
        }
    #endif
    goto skipscargv;
    scargv:;
    free(psrc_startcmd);
    if (strcmp(argv[0], psrc_main_basefilename(argv[0]))) {
        #ifndef _WIN32
        psrc_startcmd = realpath(argv[0], NULL);
        #else
        psrc_startcmd = _fullpath(NULL, argv[0], MAX_PATH);
        #endif
    } else {
        psrc_startcmd = argv[0];
    }
    skipscargv:;
    psrc_main_init();
    if (chdir(psrc_main_pathfilename(psrc_startcmd))) {psrc.displayError(PSRC_ERR, "Fatal", "Could not find resources folder"); psrc_main_cleanExit(1);};
    //psrc.sound->playMusic("resources/common/music/walk_in_the_woods.mp3");
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
    };
    unsigned int indices[] = {
         0,  1,  2,
         3,  4,  5,
         6,  7,  8,
         9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35
    };
    float vertices2[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  0.0f,  0.0f, -1.0f
    };
    float vertices3[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,  0.0f,  0.0f, -1.0f
    };
    float vertices4[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   4.0f, 4.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   4.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 4.0f,  0.0f,  0.0f, -1.0f
    };
    float vertices5[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   7.0f, 7.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   7.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 7.0f,  0.0f,  0.0f, -1.0f
    };
    unsigned int indices2[] = {
        0, 1, 3,
        1, 2, 3
    };
    psrc_gfx_obj* testobj1 = psrc.gfx->newObj((psrc_coord_3d){0, 1.5, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){1, 1, 1},
        vertices, sizeof(vertices), indices, sizeof(indices), "resources/common/textures/kekw.jpg", 256, 0);
    psrc_gfx_obj* testobj2 = psrc.gfx->newObj((psrc_coord_3d){0, 0.01, 0}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){2, 2, 2},
        vertices2, sizeof(vertices2), indices2, sizeof(indices2), "resources/common/textures/brickwall.jpg", 256, 0);
    psrc_gfx_obj* testobj3 = psrc.gfx->newObj((psrc_coord_3d){0, 0.5, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){1, 1, 1},
        vertices, sizeof(vertices), indices, sizeof(indices), "resources/common/textures/crate.jpg", 16, 0);
    psrc_gfx_obj* testobj4 = psrc.gfx->newObj((psrc_coord_3d){0, 0.75, 4}, (psrc_coord_3d){-45, 0, 0}, (psrc_coord_3d){2, 2, 2},
        vertices3, sizeof(vertices3), indices2, sizeof(indices2), NULL, 256, 0);
    psrc_gfx_light* camlight = psrc.gfx->getNextLight();
    psrc_gfx_obj* floor1 = psrc.gfx->newObj((psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices4, sizeof(vertices4), indices2, sizeof(indices2), "resources/common/textures/toxic.jpg", 0, 1);
    psrc_gfx_obj* floor2 = psrc.gfx->newObj((psrc_coord_3d){40, 0, 0}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices4, sizeof(vertices4), indices2, sizeof(indices2), "resources/common/textures/woodenfloor.jpg", 128, 0);
    psrc_gfx_obj* floor3 = psrc.gfx->newObj((psrc_coord_3d){0, 0, 40}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices5, sizeof(vertices5), indices2, sizeof(indices2), "resources/common/textures/brickwall.jpg", 0, 0);
    psrc_gfx_obj* floor4 = psrc.gfx->newObj((psrc_coord_3d){40, 0, 40}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices5, sizeof(vertices5), indices2, sizeof(indices2), "resources/common/textures/water.jpg", 0.5, 0.5);
    camlight->type = 1;
    camlight->range = 0.8;
    camlight->diffuse = (psrc_color){0.75, 0.8, 0.75};
    camlight->specular = (psrc_color){0.7, 0.8, 0.7};
    psrc_gfx_light* ambientlight = psrc.gfx->getNextLight();
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){0, 0, 0};
    ambientlight->ambient = (psrc_color){0.2, 0.4, 0.2};
    ambientlight->direction = (psrc_coord_3d){0, 180, 0};
    ambientlight->diffuse = (psrc_color){0.6, 0.9, 0.5};
    ambientlight->specular = (psrc_color){0.0, 0.0, 0.0};
    ambientlight->topCorner = (psrc_coord_3d){20.5, 20, 20.5};
    ambientlight->bottomCorner = (psrc_coord_3d){-20.5, -20, -20.5};
    psrc.gfx->updateLight(ambientlight->id);
    ambientlight = psrc.gfx->getNextLight();
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){40, 20, 0};
    ambientlight->ambient = (psrc_color){0.75, 0.7, 0.6};
    ambientlight->direction = (psrc_coord_3d){0, -180, 0};
    ambientlight->diffuse = (psrc_color){0.9, 0.7, 0.6};
    ambientlight->specular = (psrc_color){0.0, 0.0, 0.0};
    ambientlight->topCorner = (psrc_coord_3d){60.5, 20, 20.5};
    ambientlight->bottomCorner = (psrc_coord_3d){19.5, -20, -20.5};
    psrc.gfx->updateLight(ambientlight->id);
    ambientlight = psrc.gfx->getNextLight();
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){0, 20, 40};
    ambientlight->ambient = (psrc_color){0.25, 0.2, 0.1};
    ambientlight->direction = (psrc_coord_3d){0, -180, 0};
    ambientlight->diffuse = (psrc_color){0.9, 0.7, 0.6};
    ambientlight->specular = (psrc_color){0.0, 0.0, 0.0};
    ambientlight->topCorner = (psrc_coord_3d){20.5, 20, 60.5};
    ambientlight->bottomCorner = (psrc_coord_3d){-20.5, -20, 19.5};
    psrc.gfx->updateLight(ambientlight->id);
    ambientlight = psrc.gfx->getNextLight();
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){0, 20, 40};
    ambientlight->ambient = (psrc_color){0.3, 0.5, 0.7};
    ambientlight->direction = (psrc_coord_3d){0, 180, 0};
    ambientlight->diffuse = (psrc_color){0.3, 0.7, 0.8};
    ambientlight->specular = (psrc_color){0.0, 0.0, 0.0};
    ambientlight->topCorner = (psrc_coord_3d){60.5, 20, 60.5};
    ambientlight->bottomCorner = (psrc_coord_3d){19.5, -20, 19.5};
    psrc.gfx->updateLight(ambientlight->id);
    float opm = psrc_main_test_posmult;
    float orm = psrc_main_test_rotmult;
    while (!psrc.gfx->winQuit()) {
        uint64_t starttime = psrc.utime();
        psrc_main_test();
        camlight->pos = psrc.gfx->campos;
        psrc.gfx->updateLight(camlight->id);
        testobj1->rot.x = (float)glfwGetTime() * 90;
        testobj1->rot.y = (float)glfwGetTime() * 90;
        testobj1->rot.z = (float)glfwGetTime() * 90;
        testobj2->rot.z = -(float)glfwGetTime() * 45;
        psrc.gfx->renderObj(floor1);
        psrc.gfx->renderObj(floor2);
        psrc.gfx->renderObj(floor3);
        psrc.gfx->renderObj(floor4);
        psrc_main_test_renderObjAtFloors(testobj1);
        psrc_main_test_renderObjAtFloors(testobj2);
        psrc_main_test_renderObjAtFloors(testobj3);
        psrc_main_test_renderObjAtFloors(testobj4);
        psrc.gfx->updateScreen();
        uint64_t delayoffset = psrc.utime() - starttime;
        uint64_t delaytime = 1000000 / psrc.gfx->fps - delayoffset;
        if (delaytime < 1000000 / psrc.gfx->fps) psrc.wait(delaytime);
        float fpsmult = (float)(psrc.utime() - starttime) / (1000000.0f / 60.0f);
        psrc_main_test_posmult = opm * fpsmult;
        psrc_main_test_rotmult = orm * fpsmult;
    }
    psrc_main_cleanExit(0);
    return 0;
}
