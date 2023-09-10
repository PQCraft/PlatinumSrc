#ifndef ENGINE_AUDIO_H
#define ENGINE_AUDIO_H

#include "../platform.h"

#include "../psrc_game/resource.h"
#include "../psrc_aux/threading.h"

#include "../stb/stb_vorbis.h"

#if PLATFORM != PLAT_XBOX
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif

#include <stdint.h>
#include <stdbool.h>

struct audiosound_vorbisbuf {
    int off;
    int len;
    int16_t* data[2];
};
struct __attribute__((packed)) audiosound_fx {
    int posoff; // position offset in ms (based on the dist between campos and pos)
    uint16_t speedmul; // position mult in units of 256 (based on speed)
    int volmul[2]; // volume mult in units of 65536 (based on vol, camrot, and the dist between campos and pos)
};
struct __attribute__((packed)) audiosound {
    int64_t id;
    struct rc_sound* rc;
    stb_vorbis* vorbis;
    struct audiosound_vorbisbuf vorbisbuf;
    int64_t offset; // amount of samples passed in output sample rate
    uint8_t flags;
    struct {
        uint8_t paused : 1;
    } state;
    float vol;
    float speed;
    float pos[3];
    struct audiosound_fx fx[2];
};

struct audiostate {
    mutex_t lock;
    bool valid;
    SDL_AudioDeviceID output;
    int freq;
    int channels;
    struct {
        int len;
        int* data[2];
    } audbuf;
    int voices;
    struct audiosound* voicedata;
    int64_t nextid;
    float campos[3]; // for position effect
    float camrot[3];
};

enum audioopt {
    AUDIOOPT_END,
    AUDIOOPT_CAMPOS, // float, float, float
    AUDIOOPT_CAMROT, // float, float, float
    AUDIOOPT_FREQ, // int
    AUDIOOPT_VOICES, // int
};

bool initAudio(struct audiostate*);
bool startAudio(struct audiostate*);
void updateAudioConfig(struct audiostate*, ...);
void lockAudioConfig(struct audiostate*);
void unlockAudioConfig(struct audiostate*);
void stopAudio(struct audiostate*);
bool restartAudio(struct audiostate*);
void termAudio(struct audiostate*);

#define SOUNDFLAG_UNINTERRUPTIBLE (1 << 0)
#define SOUNDFLAG_LOOP (1 << 1)
#define SOUNDFLAG_FORCEMONO (1 << 2)
#define SOUNDFLAG_POSEFFECT (1 << 3)

enum soundfx {
    SOUNDFX_END,
    SOUNDFX_VOL, // float
    SOUNDFX_SPEED, // float
    SOUNDFX_POS, // float, float, float
};

int64_t playSound(struct audiostate*, struct rc_sound* rc, uint8_t flags, ... /*soundfx*/);
void changeSoundFX(struct audiostate*, int64_t, bool immediate, ...);
void changeSoundFlags(struct audiostate*, int64_t, uint8_t disable, uint8_t enable);
void stopSound(struct audiostate*, int64_t);
void pauseSound(struct audiostate*, int64_t, bool);

#endif
