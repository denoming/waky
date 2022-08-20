#include "DetectWakeWordState.hpp"

#include "StateContext.hpp"
#include "RecordingCommandState.hpp"
#include "nn/NeuralNetwork.hpp"
#include "audio-processor/AudioProcessor.hpp"
#include "audio-input/MemsMicrophone.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - DetectWakeWorkState";

DetectWakeWordState::DetectWakeWordState(StateContext& context, MemsMicrophone& sampler)
    : State{context}
    , _sampler{sampler}
    , _network{context.network()}
    , _processor{context.processor()}
{
}

void
DetectWakeWordState::enterState()
{
    ESP_LOGI(TAG, "enterState()");
}

void
DetectWakeWordState::run()
{
    ESP_LOGD(TAG, "run()");

    auto audioData = _sampler.data();
    audioData.seek(audioData.pos() - CONFIG_JRVA_I2S_SAMPLE_RATE);
    float* inputBuffer = _network.getInputBuffer();
    _processor.getSpectrogram(audioData, inputBuffer);
    const float score = _network.predict();
    if (score > 0.9) {
        ESP_LOGD(TAG, "Detected: %.2f", score);
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