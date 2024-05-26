#pragma once

#include <SDL.h>

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
    static void fillBuffer(void*, Uint8*, int);
    void fill(float*, int);

    std::uint16_t m_frequency = 0;
    SDL_AudioDeviceID m_device = 0;
    float m_phase = 0;
};

} // namespace resl
