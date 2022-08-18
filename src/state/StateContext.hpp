#pragma once

#include "state/State.hpp"

#include <memory>

class AgentUploader;
class NeuralNetwork;
class AudioProcessor;

class StateContext {
public:
    StateContext();

    ~StateContext();

    template<typename S, typename... Args>
    void
    setState(Args&&... args);

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
    std::unique_ptr<NeuralNetwork> _neuralNetwork;
    std::unique_ptr<AudioProcessor> _audioProcessor;
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