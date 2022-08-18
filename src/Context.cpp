#include "Context.hpp"

#include "NeuralNetwork.hpp"
#include "AudioProcessor.hpp"
#include "misc/AgentUploader.hpp"

Context::Context()
    : _neuralNetwork{new NeuralNetwork}
    , _audioProcessor{new AudioProcessor}
    , _uploader{new AgentUploader}
{
    _neuralNetwork->setUp();
}

Context::~Context()
{
    _neuralNetwork->tearDown();
}

void
Context::proceed()
{
    assert(_state);
    _state->run();
}

NeuralNetwork&
Context::network()
{
    assert(_neuralNetwork);
    return *_neuralNetwork;
}

AudioProcessor&
Context::processor()
{
    assert(_audioProcessor);
    return *_audioProcessor;
}

AgentUploader&
Context::uploader()
{
    assert(_uploader);
    return *_uploader;
}
