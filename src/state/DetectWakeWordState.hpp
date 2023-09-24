#pragma once

#include "State.hpp"

class StateContext;
class NeuralNetwork;
class AudioProcessor;
class MemsMicrophone;

class DetectWakeWordState final : public State {
public:
    DetectWakeWordState(StateContext& context, MemsMicrophone& sampler);

    ~DetectWakeWordState() final;

    void
    enterState() final;

    void
    run() final;

    void
    exitState() final;

private:
    MemsMicrophone& _sampler;
    NeuralNetwork& _network;
    AudioProcessor& _processor;
};