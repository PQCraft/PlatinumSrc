#include "psrc.h"

#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include <limits.h>

#ifndef MAX_PATH
#define MAX_PATH 32768
#endif

psrc_main psrc;

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

char* psrc_main_getTextFile(char* name) {
    struct stat fnst;
    stat(name, &fnst);
    if (!S_ISREG(fnst.st_mode)) return NULL;
    FILE* file = fopen(name, "r");
    if (!file) return NULL;
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

void psrc_main_init() {
    signal(SIGINT, psrc_main_cleanExitSig);
    psrc = (psrc_main){psrc_main_displayError, psrc_main_wait, psrc_main_utime, psrc_main_randfloat, psrc_main_getTextFile, NULL, NULL, false};
    if (!(psrc.sound = psrc_sound_init())) psrc_main_cleanExit(1);
    if (!(psrc.gfx = psrc_gfx_init())) psrc_main_cleanExit(1);
}

char* psrc_startcmd = NULL;

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
                size_t cb = CB_BUF_SIZE;
                sysctl(mib, 4, psrc_startcmd, &cb, NULL, 0);
                psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
                char* tmpstartcmd = realpath(psrc_startcmd, NULL);
                swap(psrc_startcmd, tmpstartcmd);
                nfree(tmpstartcmd);
                psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
            #endif
        #else
            uint32_t tmpsize = CB_BUF_SIZE;
            if (_NSGetExecutablePath(psrc_startcmd, &tmpsize)) {
                goto scargv;
            }
            char* tmpstartcmd = realpath(psrc_startcmd, NULL);
            swap(psrc_startcmd, tmpstartcmd);
            nfree(tmpstartcmd);
            psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
        #endif
    #else
        if (GetModuleFileName(NULL, psrc_startcmd, CB_BUF_SIZE)) {
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
    chdir(psrc_main_pathfilename(psrc_startcmd));
    chdir("./resources/");
    psrc_main_init();
    //psrc.sound->playMusic("common/sounds/test.mp3");
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // top left 
    };
    float vertices2[] = {
        // positions          // colors           // texture coords
         0.8f,  0.8f, 0.1f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.8f, -0.8f, 0.1f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.8f, -0.8f, 0.1f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.8f,  0.8f, 0.1f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    psrc_gfx_obj* testobj = psrc.gfx->newObj(PSRC_GFX_DEFAULT_COORD_3D, PSRC_GFX_DEFAULT_COORD_3D,
        vertices, sizeof(vertices), indices, sizeof(indices), "common/textures/crate.jpg");
    psrc_gfx_obj* testobj2 = psrc.gfx->newObj(PSRC_GFX_DEFAULT_COORD_3D, PSRC_GFX_DEFAULT_COORD_3D,
        vertices2, sizeof(vertices2), indices, sizeof(indices), "common/textures/brickwall.bmp");
    while (!psrc.gfx->winQuit()) {
        psrc.gfx->render(testobj);
        psrc.gfx->render(testobj2);
        psrc.gfx->updateScreen();
        psrc.wait(1000000 / 60);
    }
    psrc_main_cleanExit(0);
    return 0;
}
