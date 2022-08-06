#pragma once

#include "State.hpp"
#include "HttpClient.hpp"

#include <chrono>
#include <memory>

class MemsMicrophone;

class RecordingCommandState : public State {
public:
    RecordingCommandState(MemsMicrophone& sampler);

    void
    enterState() override;

    bool
    run() override;

    void
    exitState() override;

private:
    MemsMicrophone& _sampler;
    long _lastAudioPosition;
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::duration _elapsedTime;
    std::unique_ptr<HttpClient> _client;
};