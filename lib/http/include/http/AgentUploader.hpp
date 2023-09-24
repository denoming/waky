#pragma once

#include "audio-input/AudioDataAccessor.hpp"
#include "http/HttpClient.hpp"
#include "http/HttpChunkSender.hpp"

class AgentUploader final : private HttpObserver {
public:
    AgentUploader();

    ~AgentUploader() final;

    [[nodiscard]] bool
    connected() const;

    [[nodiscard]] bool
    connect();

    void
    disconnect();

    [[nodiscard]] size_t
    upload(AudioDataAccessor audioData, long start, size_t count);

    [[nodiscard]] bool
    finalize(int32_t timeout);

private:
    HttpClient _client;
    HttpChunkSender _sender;
};