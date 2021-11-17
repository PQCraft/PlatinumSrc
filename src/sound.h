#ifndef PSRC_SOUND

#define PSRC_SOUND

typedef struct {
    int music_fadeIn;
    int music_fadeOut;
    bool (*playMusic)(char*);
    bool (*waitPlayMusic)(char*);
    bool (*stopMusic)(void);
    bool (*waitStopMusic)(void);
    void (*deinit)(void);
} psrc_sound_struct;

psrc_sound_struct* psrc_sound_init();

#endif
