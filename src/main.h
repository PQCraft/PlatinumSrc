#ifndef PSRC_MAIN

#define PSRC_MAIN

#define PSRC_INF 0
#define PSRC_WRN 1
#define PSRC_ERR 2

typedef struct {
    void (*displayError)(int, char*, char*);
    void (*wait)(uint64_t);
    uint64_t (*utime)(void);
    float (*randfloat)(float, float);
    psrc_sound* sound;
    psrc_gfx* gfx;
} psrc_main;

extern psrc_main psrc;

#endif
