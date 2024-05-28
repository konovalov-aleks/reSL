#include "audio.h"

#include <SDL_error.h>

#include <cassert>
#include <iostream>

namespace resl {

namespace {

    class AudioLock {
    public:
        AudioLock(SDL_AudioDeviceID devID)
            : m_id(devID)
        {
            SDL_LockAudioDevice(m_id);
        }

        ~AudioLock()
        {
            SDL_UnlockAudioDevice(m_id);
        }

    private:
        AudioLock(const AudioLock&) = delete;
        AudioLock& operator=(const AudioLock&) = delete;

        SDL_AudioDeviceID m_id;
    };

    constexpr float g_volume = 0.4f;
    constexpr int g_sampleRate = 22050;

} // namespace

AudioDriver::AudioDriver()
{
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = g_sampleRate;
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = 64;
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
        // SDL calls the callback in a separate thread => we have to protect m_frequency
        AudioLock lock(m_device);
        m_frequency = frequency;
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
        // SDL calls the callback in a separate thread => we have to protect m_frequency
        AudioLock lock(m_device);
        frequency = m_frequency;
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
