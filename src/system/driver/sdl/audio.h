#pragma once

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_stdinc.h>

#include <atomic>
#include <cstdint>

namespace resl {

class AudioDriver {
public:
    AudioDriver();
    ~AudioDriver();

    AudioDriver(const AudioDriver&) = delete;
    AudioDriver& operator=(const AudioDriver&) = delete;

    void startSound(std::uint16_t frequency);
    void stopSound();

private:
    static void fillBuffer(void*, SDL_AudioStream*,
                           int additionalAmount, int totalAmount);
    void fill(SDL_AudioStream*, int additionalAmount, int totalAmount);

    std::atomic<std::uint16_t> m_frequency = 0;
    SDL_AudioStream* m_stream = nullptr;
    float m_phase = 0;
};

} // namespace resl
