#pragma once

#include "state/State.hpp"

#include <memory>

class NeuralNetwork;
class AudioProcessor;
class AgentUploader;
class AudioDataAccessor;

class StateContext {
public:
    ~StateContext();

    template<typename S, typename... Args>
    void
    setState(Args&&... args);

    bool
    setup();

    NeuralNetwork&
    network();

    AudioProcessor&
    processor();

    AgentUploader&
    uploader();

    void
    proceed();

private:
    std::unique_ptr<State> _state;
    std::unique_ptr<NeuralNetwork> _network;
    std::unique_ptr<AudioProcessor> _processor;
    std::unique_ptr<AgentUploader> _uploader;
};

template<typename S, typename... Args>
void
StateContext::setState(Args&&... args)
{
    if (_state) {
        _state->exitState();
    }

    _state.reset(new S{*this, std::forward<Args>(args)...});

    if (_state) {
        _state->enterState();
    }
}