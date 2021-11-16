#include "psrc.h"

#include <SDL2/SDL_mixer.h>

#define PSRC_MIX_INIT_DEFAULT (MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG)

Mix_Music* psrc_sound_music = NULL;
psrc_sound psrc_sound_struct;

void* psrc_sound_smThread(void* data) {
    (void)data;
    while (Mix_FadingMusic() != MIX_NO_FADING) {SDL_Delay(25);}
    if (!Mix_FadeOutMusic(psrc_sound_struct.music_fadeOut)) {psrc.displayError(PSRC_ERR, "Mix_FadeOutMusic", (char*)Mix_GetError());}
    if (psrc_sound_music) Mix_FreeMusic(psrc_sound_music);
    psrc_sound_music = NULL;
    return NULL;
}

bool psrc_sound_stopMusic() {
    if (!Mix_PlayingMusic()) return true;
    pthread_t ptid;
    pthread_create(&ptid, NULL, psrc_sound_smThread, NULL);
    return true;
}

bool psrc_sound_waitStopMusic() {
    while (Mix_FadingMusic() != MIX_NO_FADING) {SDL_Delay(25);}
    if (!Mix_PlayingMusic()) return true;
    if (!Mix_FadeOutMusic(psrc_sound_struct.music_fadeOut)) {psrc.displayError(PSRC_ERR, "Mix_FadeOutMusic", (char*)Mix_GetError());}
    while (Mix_FadingMusic() != MIX_NO_FADING) {SDL_Delay(25);}
    if (psrc_sound_music) Mix_FreeMusic(psrc_sound_music);
    psrc_sound_music = NULL;
    return true;
}

void* psrc_sound_mpThread(void* data) {
    while (Mix_FadingMusic() != MIX_NO_FADING) {SDL_Delay(25);}
    if (!psrc_sound_waitStopMusic()) return NULL;
    if (!(psrc_sound_music = Mix_LoadMUS(data))) {psrc.displayError(PSRC_ERR, "Mix_LoadMUS", (char*)Mix_GetError()); return NULL;}
    if (Mix_FadeInMusic(psrc_sound_music, -1, psrc_sound_struct.music_fadeIn)) {psrc.displayError(PSRC_ERR, "Mix_FadeInMusic", (char*)Mix_GetError()); return NULL;}
    return NULL;
}

bool psrc_sound_playMusic(char* filename) {
    pthread_t ptid;
    pthread_create(&ptid, NULL, psrc_sound_mpThread, filename);
    return true;
}

bool psrc_sound_waitPlayMusic(char* filename) {
    while (Mix_FadingMusic() != MIX_NO_FADING) {SDL_Delay(25);}
    if (!psrc_sound_waitStopMusic()) return false;
    if (!(psrc_sound_music = Mix_LoadMUS(filename))) {psrc.displayError(PSRC_ERR, "Mix_LoadMUS", (char*)Mix_GetError()); return false;}
    if (Mix_FadeInMusic(psrc_sound_music, -1, psrc_sound_struct.music_fadeIn)) {psrc.displayError(PSRC_ERR, "Mix_FadeInMusic", (char*)Mix_GetError()); return false;}
    return true;
}

void psrc_sound_deinit() {
    Mix_HaltMusic();
    if (psrc_sound_music) Mix_FreeMusic(psrc_sound_music);
    psrc_sound_music = NULL;
    Mix_Quit();
}

psrc_sound* psrc_sound_init() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        psrc.displayError(PSRC_ERR, "SDL_Init", (char*)SDL_GetError());
        return NULL;
    }
    if (Mix_Init(PSRC_MIX_INIT_DEFAULT) != PSRC_MIX_INIT_DEFAULT) {
        psrc.displayError(PSRC_ERR, "Mix_Init", (char*)Mix_GetError());
        return NULL;
    }
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 768);
    psrc_sound_struct = (psrc_sound){3000, 3000, psrc_sound_playMusic, psrc_sound_waitPlayMusic, psrc_sound_stopMusic, psrc_sound_waitStopMusic, psrc_sound_deinit};
    return &psrc_sound_struct;
}
