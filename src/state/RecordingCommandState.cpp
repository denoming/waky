#include "RecordingCommandState.hpp"

#include "StateContext.hpp"
#include "DetectWakeWordState.hpp"
#include "audio-input/MemsMicrophone.hpp"
#include "audio-input/MemoryPool.hpp"
#include "http/AgentUploader.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - RecordingCommandState";

static const long DEFAULT_POSITION{-1};

using namespace std::chrono;

RecordingCommandState::RecordingCommandState(StateContext& context, MemsMicrophone& sampler)
    : State{context}
    , _sampler{sampler}
    , _uploader{context.uploader()}
    , _lastAudioPosition{DEFAULT_POSITION}
{
}

void
RecordingCommandState::enterState()
{
    ESP_LOGI(TAG, "enterState()");

    _lastAudioPosition = DEFAULT_POSITION;
    _startTime = steady_clock::now();
    _elapsedTime = {};

    ESP_LOGD(TAG, "Create connection to backend");
    _uploader.connect();
}

void
RecordingCommandState::run()
{
    ESP_LOGD(TAG, "run()");

    if (!_uploader.connected()) {
        ESP_LOGE(TAG, "No connection with backend");
        return context().setState<DetectWakeWordState>(_sampler);
    }

    auto audioData = _sampler.data();
    if (_lastAudioPosition == DEFAULT_POSITION) {
        ESP_LOGD(TAG, "Initialize the last audio position");
        _lastAudioPosition = audioData.pos() - CONFIG_JRVA_I2S_SAMPLE_RATE;
    }

    static const auto capacity = MemoryPool::capacity();
    long sampleCount = (audioData.pos() - _lastAudioPosition + capacity) % capacity;

    if (sampleCount > 0) {
        ESP_LOGD(TAG, "Upload <%ld> samples to backend", sampleCount);
        _lastAudioPosition = _uploader.upload(audioData, _lastAudioPosition, sampleCount);

        const auto now = steady_clock::now();
        _elapsedTime += now - _startTime;
        _startTime = now;
        if (duration_cast<milliseconds>(_elapsedTime) > milliseconds{2000}) {
            ESP_LOGD(TAG, "Get recognition result");
            const bool result = _uploader.finalize(3000);
            ESP_LOGD(TAG, "Recognition result: %s", result ? "true" : "false");
#ifdef DEBUG
            printStackInfo(TAG, "After audio uploading");
#endif
            return context().setState<DetectWakeWordState>(_sampler);
        }
    }
}

void
RecordingCommandState::exitState()
{
    ESP_LOGI(TAG, "exitState()");

    _uploader.disconnect();
}