#include "StateContext.hpp"

#include "NeuralNetwork.hpp"
#include "AudioProcessor.hpp"
#include "misc/AgentUploader.hpp"

StateContext::StateContext()
    : _neuralNetwork{new NeuralNetwork}
    , _audioProcessor{new AudioProcessor}
    , _uploader{new AgentUploader}
{
    _neuralNetwork->setUp();
}

StateContext::~StateContext()
{
    _neuralNetwork->tearDown();
}

void
StateContext::proceed()
{
    assert(_state);
    _state->run();
}

NeuralNetwork&
StateContext::network()
{
    assert(_neuralNetwork);
    return *_neuralNetwork;
}

AudioProcessor&
StateContext::processor()
{
    assert(_audioProcessor);
    return *_audioProcessor;
}

AgentUploader&
StateContext::uploader()
{
    assert(_uploader);
    return *_uploader;
}
