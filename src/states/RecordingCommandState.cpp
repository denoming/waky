#include "RecordingCommandState.hpp"

#include "MemsMicrophone.hpp"
#include "MemoryPool.hpp"
#include "misc/AgentUploader.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - RecordingCommandState";

static const long DEFAULT_POSITION{-1};

using namespace std::chrono;

RecordingCommandState::RecordingCommandState(MemsMicrophone& sampler)
    : _sampler{sampler}
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

    ESP_LOGD(TAG, "Initialize agent uploader");
    _uploader.reset(new AgentUploader);

    ESP_LOGD(TAG, "Create connection to backend");
    _uploader->connect();
}

bool
RecordingCommandState::run()
{
    ESP_LOGD(TAG, "run()");

    if (!_uploader->connected()) {
        ESP_LOGE(TAG, "No connection with backend");
        return true;
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
        _lastAudioPosition = _uploader->upload(audioData, _lastAudioPosition, sampleCount);

        const auto now = steady_clock::now();
        _elapsedTime += now - _startTime;
        _startTime = now;
        if (duration_cast<milliseconds>(_elapsedTime) > milliseconds{2000}) {
            ESP_LOGD(TAG, "Get recognition result");
            const bool result = _uploader->finalize(3000);
            ESP_LOGD(TAG, "Recognition result: %s", result ? "true" : "false");
            return true;
        }
    }

    return false;
}

void
RecordingCommandState::exitState()
{
    ESP_LOGI(TAG, "exitState()");

    if (_uploader) {
        _uploader->disconnect();
    }

    _uploader.reset();
}