#include "RecordingCommandState.hpp"

#include "MemsMicrophone.hpp"
#include "MemoryPool.hpp"
#include "HttpChunkSender.hpp"

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

    _client.reset(new HttpClient{"192.168.1.20", 8000, "/speech"});
    _client->setHeader(Http::Field::Expect, "100-continue");
    _client->setHeader(Http::Field::UserAgent, "J.A.R.V.I.S Agent");
    _client->setMethod(HTTP_METHOD_POST);
    _client->setTimeout(8000);
    _client->connect(HttpClient::kChunkConnection);
}

bool
RecordingCommandState::run()
{
    ESP_LOGI(TAG, "run()");

    if (!_client->connected()) {
        ESP_LOGE(TAG, "No connection with recognizer backend");
        return true;
    }

    auto buffer = _sampler.buffer();
    if (_lastAudioPosition == DEFAULT_POSITION) {
        ESP_LOGI(TAG, "Initialize the last audio position");
        _lastAudioPosition = buffer.pos() - CONFIG_JRVA_I2S_SAMPLE_RATE;
    }

    static const int32_t capacity = MemoryPool::capacity();
    long sampleCount = (buffer.pos() - _lastAudioPosition + capacity) % capacity;
    if (sampleCount > 0) {
        ESP_LOGI(TAG, "The <%ld> samples is ready to send", sampleCount);

        HttpChunkSender sender{*_client};
        sender.startChunk(sampleCount * sizeof(int16_t));

        int16_t chunk[500];
        buffer.seek(_lastAudioPosition);
        while (sampleCount > 0) {
            for (int i = 0; i < sampleCount && i < 500; ++i) {
                chunk[i] = buffer.next();
            }
            ESP_LOGI(TAG, "Send the next chunk: %ld", sampleCount);
            sender.writeChunk(reinterpret_cast<const char*>(&chunk[0]),
                              std::min(sampleCount, 500l) * sizeof(int16_t));
            sampleCount -= 500;
        }
        _lastAudioPosition = buffer.pos();
        sender.endChunk();

        const auto now = steady_clock::now();
        _elapsedTime += now - _startTime;
        _startTime = now;
        if (duration_cast<milliseconds>(_elapsedTime) > milliseconds{2000}) {
            sender.finalize();
            ESP_LOGI(TAG, "Analize recognition result");
            // recognizer->getResult()
            vTaskDelay(pdMS_TO_TICKS(100));
            return true;
        }
    }

    return false;
}

void
RecordingCommandState::exitState()
{
    ESP_LOGI(TAG, "exitState()");

    if (_client) {
        _client->disconnect();
        _client->cleanup();
    }

    _client.reset();
}