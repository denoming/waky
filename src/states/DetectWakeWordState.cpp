#include "DetectWakeWordState.hpp"

#include "NeuralNetwork.hpp"
#include "AudioProcessor.hpp"
#include "MemsMicrophone.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - DetectWakeWorkState";

DetectWakeWordState::DetectWakeWordState(MemsMicrophone& sampler)
    : _sampler{sampler}
{
}

void
DetectWakeWordState::enterState()
{
    ESP_LOGI(TAG, "enterState()");

    _nn.reset(new NeuralNetwork);
    if (!_nn->setUp()) {
        ESP_LOGE(TAG, "Failed to set-up neural network");
        vTaskSuspend(NULL);
    }

    _audioProcessor.reset(new AudioProcessor);
}

bool
DetectWakeWordState::run()
{
    ESP_LOGI(TAG, "run()");

    auto buffer = _sampler.buffer();
    buffer.seek(buffer.pos() - CONFIG_JRVA_I2S_SAMPLE_RATE);
    float* inputBuffer = _nn->getInputBuffer();
    _audioProcessor->getSpectrogram(buffer, inputBuffer);
    const float output = _nn->predict();
    if (output > 0.9) {
        ESP_LOGI(TAG, "Detected: %.2f", output);
        return true;
    }

    return false;
}

void
DetectWakeWordState::exitState()
{
    ESP_LOGI(TAG, "exitState()");

    if (_nn) {
        _nn->tearDown();
    }

    _nn.reset();
    _audioProcessor.reset();
}