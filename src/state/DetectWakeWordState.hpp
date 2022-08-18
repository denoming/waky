#pragma once

#include "State.hpp"

class StateContext;
class NeuralNetwork;
class AudioProcessor;
class MemsMicrophone;

class DetectWakeWordState : public State {
public:
    DetectWakeWordState(StateContext& context, MemsMicrophone& sampler);

    void
    enterState() override;

    void
    run() override;

    void
    exitState() override;

private:
    MemsMicrophone& _sampler;
    NeuralNetwork& _network;
    AudioProcessor& _processor;
};