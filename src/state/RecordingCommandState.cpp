#include "RecordingCommandState.hpp"

#include "StateContext.hpp"
#include "DetectWakeWordState.hpp"
#include "audio-input/MemsMicrophone.hpp"
#include "audio-input/MemoryPool.hpp"
#include "http/AgentUploader.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

#include <esp_log.h>

static const char* TAG = "WAKY[RecordingCommandState]";

static const long DEFAULT_POSITION{-1};

using namespace std::chrono;

RecordingCommandState::RecordingCommandState(StateContext& context, MemsMicrophone& sampler)
    : State{context}
    , _sampler{sampler}
    , _uploader{context.uploader()}
    , _lastAudioPosition{DEFAULT_POSITION}
{
}

RecordingCommandState::~RecordingCommandState() = default;

void
RecordingCommandState::enterState()
{
    ESP_LOGI(TAG, "enterState()");

    _lastAudioPosition = DEFAULT_POSITION;
    _startTime = steady_clock::now();
    _elapsedTime = {};

#ifdef DEBUG
    ESP_LOGD(TAG, "Create connection to backend");
#endif
    if (not _uploader.connect()) {
        ESP_LOGE("TAG", "Unable to connect");
    }
}

void
RecordingCommandState::run()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "run()");
#endif

    if (!_uploader.connected()) {
        ESP_LOGE(TAG, "No connection with backend");
        return context().setState<DetectWakeWordState>(_sampler);
    }

    auto audioData = _sampler.data();
    if (_lastAudioPosition == DEFAULT_POSITION) {
#ifdef DEBUG
        ESP_LOGD(TAG, "Initialize the last audio position");
#endif
        _lastAudioPosition = audioData.pos() - CONFIG_WAKY_MEMPOOL_BUFFER_SIZE;
    }

    static const auto capacity = MemoryPool::capacity();
    long sampleCount = (audioData.pos() - _lastAudioPosition + capacity) % capacity;

    if (sampleCount > 0) {
#ifdef DEBUG
        ESP_LOGD(TAG, "Upload <%ld> samples to backend", sampleCount);
#endif
        _lastAudioPosition = _uploader.upload(audioData, _lastAudioPosition, sampleCount);

        const auto now = steady_clock::now();
        _elapsedTime += now - _startTime;
        _startTime = now;
        if (duration_cast<milliseconds>(_elapsedTime) > milliseconds{2000}) {
#ifdef DEBUG
            ESP_LOGD(TAG, "Get recognition result");
            const bool result = _uploader.finalize(3000);
            ESP_LOGD(TAG, "Recognition result: %s", result ? "true" : "false");
            printStackInfo(TAG, "After audio uploading");
#else
            std::ignore = _uploader.finalize(3000);
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