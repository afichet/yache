#include <beeper.h>
#include <cmath>
#include <stdexcept>


Beeper::Beeper(float freq, float duration)
    : m_freq(freq)
    , m_duration_left(duration)
    , m_attack(1.f/30.f)
    , m_sustain(1.f/30.f)
    , m_decay(0.1f)
{
    SDL_AudioSpec specs_desired;
    SDL_zero(specs_desired);

    specs_desired.freq     = 44100;
    specs_desired.format   = AUDIO_S16SYS;
    specs_desired.samples  = 2048;
    specs_desired.channels = 1;
    specs_desired.callback = Beeper::audio_cb;
    specs_desired.userdata = this;

    m_audio_dev = SDL_OpenAudioDevice(
        NULL, 0,
        &specs_desired,
        &m_specs,
        0
    );

    if (!m_audio_dev) {
        throw std::runtime_error(SDL_GetError());
    }

    if (specs_desired.format != m_specs.format) {
        throw std::runtime_error(SDL_GetError());
    }
}


Beeper::~Beeper()
{
    SDL_CloseAudioDevice(m_audio_dev);
}


void Beeper::setPaused(int is_paused)
{
    SDL_PauseAudioDevice(m_audio_dev, is_paused);
}


void Beeper::setDurationLeft(float duration)
{
    m_duration_left = duration;
}


float Beeper::minDuration() const 
{
    return m_attack + m_sustain + m_decay;
}


void Beeper::audio_cb(void * userdata, Uint8* stream, int len)
{
    Beeper* b = (Beeper*)userdata;

    const float max_amplitude = 50000.f;
    const float sustain_amplitude = 35000.f;
    const float sample_duration = 1.f / (float)b->m_specs.freq;

    SDL_LockAudioDevice(b->m_audio_dev);

    Sint16* samples = (Sint16*)stream;

    for (int i = 0; i < len / 2; i++) {
        if (b->m_duration_left > 0.f) {
            const float t = (float)(i + b->m_samples_gen) / (float)(b->m_specs.freq);
            
            float amplitude = 0.f;

            if (b->m_duration_left <= b->m_decay) {
                // End of the note
                const float a = b->m_duration_left / b->m_decay;
                amplitude = a * sustain_amplitude;
            } 
            else if (b->m_duration_played < b->m_attack) {
                // Start of the note
                const float a = b->m_duration_played / b->m_attack;
                amplitude = a * max_amplitude;
            }
            else if (b->m_duration_played - b->m_attack < b->m_sustain) {
                // Sustain
                const float a = (b->m_duration_played - b->m_attack) / (b->m_sustain);
                amplitude = max_amplitude + a * (sustain_amplitude - max_amplitude);
            }
            else {
                // Permanent mode
                amplitude = sustain_amplitude;
            } 

            samples[i] = amplitude * std::sin(2. * M_PI * t * b->m_freq);

            b->m_duration_left -= sample_duration;
            b->m_duration_played += sample_duration;
        } else {
            samples[i] = 0;
            b->m_duration_played = 0;
            b->setPaused(1);
        }

        // Debug
        // std::cout << i + b->m_samples_gen << " " << samples[i] << std::endl;
    }

    b->m_samples_gen = (b->m_samples_gen + b->m_specs.samples) % b->m_specs.freq;
    // Replace by this if you use the debug printf
    // b->m_samples_gen = (b->m_samples_gen + b->m_specs.samples);

    SDL_UnlockAudioDevice(b->m_audio_dev);
}