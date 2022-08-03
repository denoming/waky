#include "HttpClient.hpp"

#include <esp_log.h>
#include <esp_err.h>

static const char* TAG = "ESP32 JRVA - HttpClient";

HttpClient::HttpClient()
    : HttpClient{"127.0.0.1", 80, "/"}
{
}

HttpClient::HttpClient(const char* host, const char* path)
    : HttpClient{host, 80, path}
{
}

HttpClient::HttpClient(const char* host, unsigned short port, const char* path)
    : _connected{false}
    , _handle{nullptr}
{
    esp_http_client_config_t config = {};
    config.transport_type = HTTP_TRANSPORT_OVER_TCP;
    config.host = host;
    config.port = port;
    config.path = path;
    config.user_data = this;
    config.event_handler = eventHandler;
    _handle = esp_http_client_init(&config);
    assert(_handle != nullptr);
}

bool
HttpClient::connected() const
{
    return _connected;
}

bool
HttpClient::connect(int writeLength)
{
    const auto error = esp_http_client_open(_handle, writeLength);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect: %s", esp_err_to_name(error));
        return false;
    }
    return true;
}

bool
HttpClient::disconnect()
{
    const auto error = esp_http_client_close(_handle);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disconnect: %s", esp_err_to_name(error));
        return false;
    }
    return true;
}

bool
HttpClient::cleanup()
{
    const auto error = esp_http_client_cleanup(_handle);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to cleanup: %s", esp_err_to_name(error));
        return false;
    }
    return true;
}

int
HttpClient::fetchHeaders()
{
    return esp_http_client_fetch_headers(_handle);
}

int
HttpClient::write(const char* data, size_t size)
{
    assert(data != nullptr);
    assert(size > 0);
    return esp_http_client_write(_handle, data, size);
}

bool
HttpClient::setMethod(esp_http_client_method_t method)
{
    const auto error = esp_http_client_set_method(_handle, method);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set method: %s", esp_err_to_name(error));
        return false;
    }
    return true;
}

bool
HttpClient::setHeader(Http::Field field, const char* value)
{
    return esp_http_client_set_header(_handle, Http::toString(field), value);
}

bool
HttpClient::setUrl(const char* value)
{
    const auto error = esp_http_client_set_url(_handle, value);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set url: %s", esp_err_to_name(error));
        return false;
    }
    return true;
}

bool
HttpClient::setTimeout(unsigned int value)
{
    const auto error = esp_http_client_set_timeout_ms(_handle, value);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set timeout: %s", esp_err_to_name(error));
        return false;
    }
    return true;
}

int
HttpClient::getStatusCode()
{
    return esp_http_client_get_status_code(_handle);
}

void
HttpClient::onConnect()
{
    _connected = true;
}

void
HttpClient::onDisconnect()
{
    _connected = false;
}

esp_err_t
HttpClient::eventHandler(esp_http_client_event_t* event)
{
    auto* self = static_cast<HttpClient*>(event->user_data);
    assert(self != nullptr);

    switch (event->event_id) {
    case HTTP_EVENT_ON_CONNECTED:
        self->onConnect();
        break;
    case HTTP_EVENT_DISCONNECTED:
        self->onDisconnect();
        break;
    default:
        break;
    }

    return ESP_OK;
}