#include "DetectWakeWordState.hpp"

#include "StateContext.hpp"
#include "RecordingCommandState.hpp"
#include "nn/NeuralNetwork.hpp"
#include "audio-processor/AudioProcessor.hpp"
#include "audio-input/MemsMicrophone.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - DetectWakeWorkState";

DetectWakeWordState::DetectWakeWordState(StateContext& context, MemsMicrophone& sampler)
    : State{context}
    , _sampler{sampler}
    , _network{context.network()}
    , _processor{context.processor()}
{
}

DetectWakeWordState::~DetectWakeWordState() = default;

void
DetectWakeWordState::enterState()
{
    ESP_LOGI(TAG, "enterState()");
}

void
DetectWakeWordState::run()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "run()");
#endif

    auto audioData = _sampler.data();
    audioData.seek(audioData.pos() - CONFIG_WAKY_MEMPOOL_BUFFER_SIZE);
    float* inputBuffer = _network.getInputBuffer();
    _processor.getSpectrogram(audioData, inputBuffer);
    const float score = _network.predict();
    if (score >= 0.9) {
        ESP_LOGI(TAG, "Detected: %.2f", score);
#ifdef DEBUG
        printStackInfo(TAG, "After wake word detected");
#endif
        return context().setState<RecordingCommandState>(_sampler);
    }
}

void
DetectWakeWordState::exitState()
{
    ESP_LOGI(TAG, "exitState()");
}