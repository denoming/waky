#pragma once

#include "AudioDataAccessor.hpp"
#include "HttpClient.hpp"
#include "HttpChunkSender.hpp"

class AgentUploader : private HttpObserver {
public:
    AgentUploader();

    bool
    connected() const;

    bool
    connect();

    void
    disconnect();

    std::size_t
    upload(AudioDataAccessor audioData, long start, std::size_t count);

    bool
    finalize(std::int32_t timeout);

private:
    HttpClient _client;
    HttpChunkSender _sender;
};