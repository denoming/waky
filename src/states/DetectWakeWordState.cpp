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

    ESP_LOGD(TAG, "Initialize netural network");
    _nn.reset(new NeuralNetwork);
    if (!_nn->setUp()) {
        ESP_LOGE(TAG, "Failed to set-up neural network");
        vTaskSuspend(NULL);
    }

    ESP_LOGD(TAG, "Initialize audio processor");
    _audioProcessor.reset(new AudioProcessor);
}

bool
DetectWakeWordState::run()
{
    ESP_LOGD(TAG, "run()");

    auto audioData = _sampler.data();
    audioData.seek(audioData.pos() - CONFIG_JRVA_I2S_SAMPLE_RATE);
    float* inputBuffer = _nn->getInputBuffer();
    _audioProcessor->getSpectrogram(audioData, inputBuffer);
    const float score = _nn->predict();
    if (score > 0.9) {
        ESP_LOGD(TAG, "Detected: %.2f", score);
        return true;
    }

    return false;
}

void
DetectWakeWordState::exitState()
{
    ESP_LOGI(TAG, "exitState()");

    ESP_LOGD(TAG, "Deinitialize netural network");
    if (_nn) {
        _nn->tearDown();
    }
    _nn.reset();

    ESP_LOGD(TAG, "Deinitialize audio processor");
    _audioProcessor.reset();
}