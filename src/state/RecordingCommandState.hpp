#pragma once

#include "State.hpp"

#include <chrono>

class MemsMicrophone;
class AgentUploader;

class RecordingCommandState final : public State {
public:
    RecordingCommandState(StateContext& context, MemsMicrophone& sampler);

    ~RecordingCommandState() final;

    void
    enterState() final;

    void
    run() final;

    void
    exitState() final;

private:
    MemsMicrophone& _sampler;
    AgentUploader& _uploader;
    long _lastAudioPosition;
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::duration _elapsedTime;
};