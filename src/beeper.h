#pragma once

#include <SDL.h>

class Beeper
{
public:
    Beeper(float freq = 440.f, float duration = 1.f);

    virtual ~Beeper();

    void setPaused(int is_paused);

    void setDurationLeft(float duration);

    float minDuration() const;

    static void audio_cb(void * userdata, Uint8* stream, int len);

private:
    float m_freq;

    float m_duration_played;
    float m_duration_left;

    float m_attack;
    float m_sustain;
    float m_decay;

    SDL_AudioSpec m_specs;
    SDL_AudioDeviceID m_audio_dev;
    int m_samples_gen;
};