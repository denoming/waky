#pragma once

#include "State.hpp"

#include <chrono>

class MemsMicrophone;
class AgentUploader;

class RecordingCommandState : public State {
public:
    RecordingCommandState(StateContext& context, MemsMicrophone& sampler);

    void
    enterState() override;

    void
    run() override;

    void
    exitState() override;

private:
    MemsMicrophone& _sampler;
    AgentUploader& _uploader;
    long _lastAudioPosition;
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::duration _elapsedTime;
};