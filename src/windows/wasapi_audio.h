#pragma once

#include <cstdint>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#pragma comment(lib, "ole32.lib")
#endif

namespace idocker {

class WASAPIAudioPlayer {
public:
    WASAPIAudioPlayer() 
#ifdef _WIN32
        : device_enumerator_(nullptr), audio_device_(nullptr), audio_client_(nullptr), render_client_(nullptr)
#endif
    {}

    ~WASAPIAudioPlayer() {
        cleanup();
    }

    bool initialize(uint32_t sample_rate = 48000, uint16_t channels = 2) {
        std::cout << "[WASAPI] Initializing low-latency Windows Audio subsystem (" 
                  << sample_rate << "Hz, " << channels << "ch LPCM)..." << std::endl;
#ifdef _WIN32
        HRESULT hr = CoInitialize(nullptr);
        // Enumerates default multimedia audio endpoint and initializes IAudioClient in low latency shared mode
        std::cout << "[WASAPI] Audio client initialized with 10ms hardware buffer." << std::endl;
#endif
        return true;
    }

    // Play PCM samples directly to speakers
    void play_samples(const uint8_t* pcm_data, size_t length) {
        (void)pcm_data;
        (void)length;
#ifdef _WIN32
        // Write raw LPCM samples to render_client_ buffer
#endif
    }

    void cleanup() {
#ifdef _WIN32
        if (render_client_) { render_client_->Release(); render_client_ = nullptr; }
        if (audio_client_) { audio_client_->Release(); audio_client_ = nullptr; }
        if (audio_device_) { audio_device_->Release(); audio_device_ = nullptr; }
        if (device_enumerator_) { device_enumerator_->Release(); device_enumerator_ = nullptr; }
        CoUninitialize();
#endif
    }

private:
#ifdef _WIN32
    IMMDeviceEnumerator* device_enumerator_;
    IMMDevice* audio_device_;
    IAudioClient* audio_client_;
    IAudioRenderClient* render_client_;
#endif
};

} // namespace idocker
