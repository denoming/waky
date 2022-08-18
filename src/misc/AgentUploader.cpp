#include "AgentUploader.hpp"

#include "HttpClient.hpp"
#include "HttpChunkSender.hpp"

#include <esp_assert.h>
#include <esp_err.h>
#include <esp_log.h>

#include <cJSON.h>

#include <algorithm>
#include <string>
#include <vector>

static const char* TAG = "ESP32 JRVA - AgentUploader";

namespace {

bool
getResult(const char* str, std::size_t len, std::string& error)
{
    assert(str != nullptr);
    assert(len > 0);

    bool output{false};
    error.clear();

    auto* jRoot = cJSON_ParseWithLength(str, len);
    if (jRoot == nullptr) {
        error.assign("Failed to parse JSON");
        return false;
    }
    const auto* jStatus = cJSON_GetObjectItemCaseSensitive(jRoot, "status");
    if (jStatus == nullptr) {
        error.assign("Invalid JSON structure");
        return false;
    }
    if (cJSON_IsBool(jStatus)) {
        output = jStatus->valueint;
    }
    if (!output) {
        const auto* jError = cJSON_GetObjectItemCaseSensitive(jRoot, "error");
        if (cJSON_IsString(jError) && jError->valuestring != nullptr) {
            error.assign(jError->valuestring);
        }
    }
    cJSON_Delete(jRoot);
    return output;
}

} // namespace

AgentUploader::AgentUploader()
    : _client{*this, CONFIG_JRVA_BACKEND_ADDR, CONFIG_JRVA_BACKEND_PORT, CONFIG_JRVA_BACKEND_PATH}
    , _sender{_client}
{
}

AgentUploader::~AgentUploader() = default;

bool
AgentUploader::connected() const
{
    return _client.connected();
}

bool
AgentUploader::connect()
{
    assert(!_client.connected());

    _client.setHeader(Http::Field::Expect, "100-continue");
    _client.setHeader(Http::Field::UserAgent, "J.A.R.V.I.S Agent");
    _client.setMethod(HTTP_METHOD_POST);
    _client.setTimeout(CONFIG_JRVA_BACKEND_TIMEOUT);

    ESP_LOGD(TAG, "Create connection to backend server");
    return _client.connect(HttpClient::kChunkConnection);
}

void
AgentUploader::disconnect()
{
    if (_client.connected()) {
        _client.disconnect();
    }
}

std::size_t
AgentUploader::upload(AudioDataAccessor audioData, long start, std::size_t count)
{
    static const std::size_t ChunkSize = CONFIG_JRVA_BACKEND_CHUNK_SIZE;

    audioData.seek(start);

    ESP_LOGD(TAG, "Start writing chunk: start<%ld>, count<%u>", start, count);
    _sender.startChunk(count * sizeof(int16_t));

    static int16_t chunk[ChunkSize];
    assert(count > 0);
    while (count > 0) {
        for (int i = 0; i < count && i < ChunkSize; ++i) {
            chunk[i] = audioData.next();
        }
        const auto n = std::min(count, ChunkSize);
        _sender.writeChunk(reinterpret_cast<const char*>(&chunk[0]), n * sizeof(int16_t));
        count -= n;
    }

    ESP_LOGD(TAG, "End writing chunk: start<%ld>", start);
    _sender.endChunk();

    return audioData.pos();
}

bool
AgentUploader::finalize(std::int32_t timeout)
{
    static auto kWaitingDataTimeout = pdMS_TO_TICKS(300);

    ESP_LOGD(TAG, "Finalize sending of chunks");
    _sender.finalize();

    assert(timeout > 0);
    while (timeout > 0) {
        vTaskDelay(kWaitingDataTimeout);

        int len = _client.fetchHeaders();
        if (len == 0) {
            continue;
        }
        if (len == -1 || len == -ESP_ERR_HTTP_EAGAIN) {
            ESP_LOGD(TAG, "Stream fetch headers error: %d", len);
            break;
        }

        int statusCode = _client.getStatusCode();
        ESP_LOGD(TAG, "Stream status: len<%d>, code<%d>", len, statusCode);

        ESP_LOGD(TAG, "Read result from backend");
        std::vector<char> str(len, '\0');
        len = _client.read(str.data(), len);
        if (len > 0) {
            std::string error;
            const bool status = getResult(str.data(), len, error);
            if (!status) {
                ESP_LOGE(TAG, "Get result failed: %s", error.data());
            }
            return status;
        } else {
            ESP_LOGD(TAG, "Stream read error: %d", len);
            return false;
        }

        timeout -= 300;
    }

    return false;
}