#pragma once

#include <typeindex>

class HttpClient;

class HttpChunkSender {
public:
    HttpChunkSender(HttpClient& client);

    bool
    send(const char* data, std::size_t size);

    bool
    send(const char* data);

    bool
    startChunk(std::size_t size);

    bool
    writeChunk(const char* data, std::size_t size);

    bool
    endChunk();

    bool
    finalize();

private:
    HttpClient& _client;
};