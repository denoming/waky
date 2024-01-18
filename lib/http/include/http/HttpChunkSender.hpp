#pragma once

#include <typeindex>

class HttpClient;

class HttpChunkSender {
public:
    explicit HttpChunkSender(HttpClient& client);

    [[nodiscard]] bool
    send(const char* data, std::size_t size);

    [[nodiscard]] bool
    send(const char* data);

    [[nodiscard]] bool
    startChunk(std::size_t size);

    [[nodiscard]] bool
    writeChunk(const char* data, std::size_t size);

    [[nodiscard]] bool
    endChunk();

    [[nodiscard]] bool
    finalize();

private:
    HttpClient& _client;
};