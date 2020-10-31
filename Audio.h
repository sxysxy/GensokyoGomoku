/*
    Audio.h
        提供对BGM和SoundEffect基本的支持
                        by石响宇 2020.07.07
*/


#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <stdexcept>

class SoundEffect;
class Music;

class Audio {
public:
    static void init();

    static void playBGM(Music* music, bool loop = false, int volumn = -1, double pos = -1);

    static void playSE(SoundEffect* se);

    static void pauseBGM();
    static void resumeBGM();
    static void fadeOutBGM(int fadeTime = 200);
    static void stopBGM();
    static bool isPlayingBGM();
};

class Music {
    Mix_Music* music = nullptr;    
public:
    Music(const std::string& filename) {
        music = Mix_LoadMUS(filename.c_str());
        if (!music) {
            throw std::runtime_error(std::string("Could not load ") + filename);
        }
    }
    ~Music() {
        if (music) {
            Mix_FreeMusic(music);
            music = nullptr;
        }
    }
    friend void Audio::playBGM(Music*, bool, int, double);
};

class SoundEffect {
    Mix_Chunk* chunk = nullptr;
public:
    SoundEffect(const std::string& filename) {
        chunk = Mix_LoadWAV(filename.c_str());
        if (!chunk) {
            throw std::runtime_error(std::string("Could not load ") + filename);
        }
        setVolumn();
    }
    ~SoundEffect() {
        if (chunk) {
            Mix_FreeChunk(chunk);
            chunk = nullptr;
        }
    }
    void setVolumn(int v = 100) {
        if(chunk)
            Mix_VolumeChunk(chunk, v);
    }

    friend void Audio::playSE(SoundEffect*);
};



