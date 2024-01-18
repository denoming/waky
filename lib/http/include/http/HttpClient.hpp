#pragma once

#include "http/Http.hpp"
#include "http/HttpObserver.hpp"
#include "http/Prinatble.hpp"

#include <esp_http_client.h>

class HttpClient : public Printable {
public:
    static const int kChunkConnection = -1;
    static const int kReadOnlyConnection = 0;

    explicit HttpClient(HttpObserver& observer);

    HttpClient(HttpObserver& observer, const char* host, const char* path);

    HttpClient(HttpObserver& observer, const char* host, unsigned short port, const char* path);

    ~HttpClient();

    [[nodiscard]] bool
    connected() const;

    [[nodiscard]] bool
    connect(int writeLength = kReadOnlyConnection);

    [[nodiscard]] bool
    disconnect();

    [[nodiscard]] int
    fetchHeaders();

    [[nodiscard]] int
    read(char* buffer, size_t size);

    [[nodiscard]] int
    write(const char* data, size_t size) override;

    [[nodiscard]] bool
    setMethod(esp_http_client_method_t method);

    [[nodiscard]] bool
    setHeader(Http::Field field, const char* value);

    [[nodiscard]] bool
    setUrl(const char* value);

    [[nodiscard]] bool
    setTimeout(unsigned int value);

    [[nodiscard]] int
    getStatusCode();

protected:
    HttpObserver&
    observer();

private:
    void
    setConnectedStatus(bool status);

    static esp_err_t
    eventHandler(esp_http_client_event_t* event);

private:
    HttpObserver& _observer;
    bool _connected;
    esp_http_client_handle_t _handle;
};
