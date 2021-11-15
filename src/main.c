#include "psrc.h"

#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

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

void psrc_main_cleanExit(int code) {
    free(psrc_main_getErrorText_text);
    if (psrc.sound && psrc.sound->deinit) psrc.sound->deinit();
    if (psrc.gfx && psrc.gfx->deinit) psrc.gfx->deinit();
    SDL_Quit();
    exit(code);
}

void psrc_main_cleanExitSig(int sig) {
    (void)sig;
    psrc_main_cleanExit(0);
}

void psrc_main_init() {
    signal(SIGINT, psrc_main_cleanExitSig);
    psrc = (psrc_main){psrc_main_displayError, psrc_main_wait, psrc_main_utime, psrc_main_randfloat, NULL, NULL};
    if (!(psrc.sound = psrc_sound_init())) psrc_main_cleanExit(1);
    psrc.sound->playMusic("test.mp3");
    if (!(psrc.gfx = psrc_gfx_init())) psrc_main_cleanExit(1);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    psrc_main_init();
    psrc_main_cleanExit(0);
    return 0;
}
