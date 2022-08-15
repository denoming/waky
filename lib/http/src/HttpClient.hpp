#pragma once

#include "Http.hpp"
#include "HttpObserver.hpp"
#include "Prinatble.hpp"

#include <esp_http_client.h>

class HttpClient : public Printable {
public:
    static const int kChunkConnection = -1;
    static const int kReadOnlyConnection = 0;

    HttpClient(HttpObserver& observer);

    HttpClient(HttpObserver& observer, const char* host, const char* path);

    HttpClient(HttpObserver& observer, const char* host, unsigned short port, const char* path);

    ~HttpClient();

    bool
    connected() const;

    bool
    connect(int writeLength = kReadOnlyConnection);

    bool
    disconnect();

    int
    fetchHeaders();

    int
    read(char* buffer, size_t size);

    int
    write(const char* data, size_t size) override;

    bool
    setMethod(esp_http_client_method_t method);

    bool
    setHeader(Http::Field field, const char* value);

    bool
    setUrl(const char* value);

    bool
    setTimeout(unsigned int value);

    int
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
