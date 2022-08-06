#pragma once

#include "State.hpp"

#include <memory>

class NeuralNetwork;
class AudioProcessor;
class MemsMicrophone;

class DetectWakeWordState : public State {
public:
    DetectWakeWordState(MemsMicrophone& sampler);

    void enterState() override;

    bool run() override;

    void exitState() override;

private:
    MemsMicrophone& _sampler;
    std::unique_ptr<NeuralNetwork> _nn;
    std::unique_ptr<AudioProcessor> _audioProcessor;
};