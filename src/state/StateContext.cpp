#include "StateContext.hpp"

#include "nn/NeuralNetwork.hpp"
#include "audio-processor/AudioProcessor.hpp"
#include "audio-input/AudioDataAccessor.hpp"
#include "http/AgentUploader.hpp"

StateContext::~StateContext() = default;

bool
StateContext::setup()
{
    _network = std::make_unique<NeuralNetwork>();
    _processor = std::make_unique<AudioProcessor>();
    _uploader = std::make_unique<AgentUploader>();

    assert(_network);
    if (!_network->setUp()) {
        return false;
    }

    return true;
}

NeuralNetwork&
StateContext::network()
{
    assert(_network);
    return *_network;
}

AudioProcessor&
StateContext::processor()
{
    assert(_processor);
    return *_processor;
}

AgentUploader&
StateContext::uploader()
{
    assert(_uploader);
    return *_uploader;
}

void
StateContext::proceed()
{
    assert(_state);
    _state->run();
}
