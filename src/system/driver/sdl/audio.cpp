#include "audio.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>

#include <cassert>
#include <iostream>

namespace resl {

namespace {

    constexpr float g_volume = 0.2f;
    constexpr int g_sampleRate = 22050;

} // namespace

AudioDriver::AudioDriver()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) [[unlikely]] {
        std::cerr << "Unable to initialize audio subsystem. Continue without sound.\n"
                  << SDL_GetError() << std::endl;
        return;
    }

    const SDL_AudioSpec spec = { SDL_AUDIO_F32LE, 1, g_sampleRate };
    m_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &fillBuffer, this);
    if (!m_stream) [[unlikely]] {
        std::cerr << "WARNING: unable to initialize an audio device! Continue without sound.\n"
                  << SDL_GetError() << std::endl;
        return;
    }
}

AudioDriver::~AudioDriver()
{
    if (m_stream)
        SDL_DestroyAudioStream(m_stream);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioDriver::startSound(std::uint16_t frequency)
{
    assert(frequency * 2 <= g_sampleRate);

    if (!m_stream) [[unlikely]]
        return;

    m_frequency.store(frequency, std::memory_order_relaxed);
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(m_stream));
}

void AudioDriver::stopSound()
{
    if (!m_stream) [[unlikely]]
        return;

    SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(m_stream));
}

void AudioDriver::fillBuffer(void* userData, SDL_AudioStream* stream, int additionalAmount, int totalAmount)
{
    AudioDriver* drv = reinterpret_cast<AudioDriver*>(userData);
    drv->fill(stream, additionalAmount, totalAmount);
}

void AudioDriver::fill(SDL_AudioStream* stream, int additionalAmount, int /* totalAmount */)
{
    if (additionalAmount <= 0)
        return;

    const int sampleCount = additionalAmount / sizeof(float);
    float* data = SDL_stack_alloc(float, sampleCount);
    if (!data) [[unlikely]]
        return;

    std::uint16_t frequency = m_frequency.load(std::memory_order_relaxed);

    const float phaseStep =
        static_cast<float>(frequency) / static_cast<float>(g_sampleRate);

    for (int i = 0; i < sampleCount; ++i) {
        data[i] = m_phase > 0.5f ? g_volume : -g_volume;
        m_phase += phaseStep;
        if (m_phase >= 1.0f)
            m_phase -= 1.0f;
    }

    SDL_PutAudioStreamData(stream, data, sampleCount * sizeof(float));
    SDL_stack_free(data);
}

} // namespace resl
