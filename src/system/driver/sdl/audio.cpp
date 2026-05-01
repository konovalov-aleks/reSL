#include "audio.h"

#include <SDL_error.h>

#include <cassert>
#include <iostream>

namespace resl {

namespace {

    constexpr float g_volume = 0.2f;
    constexpr int g_sampleRate = 22050;

} // namespace

AudioDriver::AudioDriver()
{
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = g_sampleRate;
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = 256;
    desired.callback = &fillBuffer;
    desired.userdata = this;

    m_device = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (!m_device) [[unlikely]] {
        // Failure to initialize an audio device is not critical - just continue without sound
        std::cerr << "WARNING: unable to initialize an audio device! Continue without sound.\n"
                  << SDL_GetError() << std::endl;
        return;
    }
}

AudioDriver::~AudioDriver()
{
    if (m_device)
        SDL_CloseAudioDevice(m_device);
}

void AudioDriver::startSound(std::uint16_t frequency)
{
    assert(frequency * 2 <= g_sampleRate);

    if (!m_device) [[unlikely]]
        return;

    {
        // SDL calls the callback in a separate thread
        m_frequency.store(frequency, std::memory_order_relaxed);
    }
    SDL_PauseAudioDevice(m_device, 0);
}

void AudioDriver::stopSound()
{
    if (!m_device) [[unlikely]]
        return;

    SDL_PauseAudioDevice(m_device, 1);
}

void AudioDriver::fillBuffer(void* userData, Uint8* data, int len)
{
    AudioDriver* drv = reinterpret_cast<AudioDriver*>(userData);
    drv->fill(reinterpret_cast<float*>(data), len / sizeof(float));
}

void AudioDriver::fill(float* buf, int len)
{
    std::uint16_t frequency;
    {
        // SDL calls the callback in a separate thread
        frequency = m_frequency.load(std::memory_order_relaxed);
    }

    const float phaseStep =
        static_cast<float>(frequency) / static_cast<float>(g_sampleRate);

    for (int i = 0; i < len; ++i) {
        buf[i] = m_phase > 0.5f ? g_volume : -g_volume;
        m_phase += phaseStep;
        if (m_phase >= 1.0f)
            m_phase -= 1.0f;
    }
}

} // namespace resl
