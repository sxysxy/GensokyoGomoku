#include "Audio.h"

void Audio::init() {
    SDL_Init(SDL_INIT_AUDIO);
    Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_FLAC);
    Mix_OpenAudio(44100, 0x8010, 2, 4096);
    Mix_AllocateChannels(16);
    Mix_VolumeMusic(100);
}

void Audio::playBGM(Music* music, bool loop, int volumn, double pos) {
    if (volumn > 0) {
        Mix_VolumeMusic(volumn);
    }
    if (pos > 0) {
        Mix_SetMusicPosition(pos);
        Mix_PlayMusic(music->music, loop ? -1 : 1);
    }
}

void Audio::playSE(SoundEffect* se) {
    Mix_PlayChannel(-1, se->chunk, 0);
}

void Audio::fadeOutBGM(int fadeTime) {
    Mix_FadeOutMusic(fadeTime);
}

bool Audio::isPlayingBGM() {
    return Mix_PlayingMusic();
}

void Audio::pauseBGM() {
    Mix_PauseMusic();
}

void Audio::stopBGM() {
    Mix_HaltMusic();
}

void Audio::resumeBGM() {
    Mix_ResumeMusic();
}