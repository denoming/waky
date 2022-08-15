#pragma once

#include <memory>

class MemsMicrophone;
class State;

class Context {
public:
    Context();

    void
    proceed(MemsMicrophone& sampler);

private:
    std::unique_ptr<State> _detectionState;
    std::unique_ptr<State> _recordingState;
    State* _state;
};
