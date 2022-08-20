#pragma once

#include "audio-input/AudioDataAccessor.hpp"
#include "http/HttpClient.hpp"
#include "http/HttpChunkSender.hpp"

class AgentUploader : private HttpObserver {
public:
    AgentUploader();

    ~AgentUploader();

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