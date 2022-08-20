#include "audio-processor/AudioProcessor.hpp"

#include "audio-input/AudioDataAccessor.hpp"

#include <cfloat>

namespace {

int
getInputSize(int windowSize)
{
    int size = 1;
    while (size < windowSize) {
        size <<= 1;
    }
    return size;
}

float
getMeanValue(AudioDataAccessor audioData, int audioLength)
{
    float mean{0.0};
    for (int i = 0; i < audioLength; i++) {
        mean += audioData.next();
    }
    return (mean / audioLength);
}

float
getMaxDeviationValue(AudioDataAccessor audioData, int audioLength, float mean)
{
    float max{0.0};
    for (int i = 0; i < audioLength; i++) {
        max = std::max(max, fabsf(static_cast<float>(audioData.next()) - mean));
    }
    return max;
}

} // namespace

AudioProcessor::AudioProcessor()
    : AudioProcessor{CONFIG_JRVA_I2S_SAMPLE_RATE,
                     CONFIG_JRVA_WWD_WINDOW_SIZE,
                     CONFIG_JRVA_WWD_STEP_SIZE,
                     CONFIG_JRVA_WWD_POOLING_SIZE}
{
}

AudioProcessor::AudioProcessor(int audioLength, int windowSize, int stepSize, int poolingSize)
    : _audioLength{audioLength}
    , _windowSize{windowSize}
    , _stepSize{stepSize}
    , _poolingSize{poolingSize}
    , _fftSize{getInputSize(windowSize)}
    , _energySize{_fftSize / 2 + 1}
    , _pooledEnergySize{static_cast<int>(ceilf(_energySize / static_cast<float>(poolingSize)))}
    , _hammingWindow{windowSize}
{
    _fftInput.reset(new float[_fftSize]);
    _fftOutput.reset(new kiss_fft_cpx[_energySize]);
    _energy.reset(new float[_energySize]);

    _cfg = kiss_fftr_alloc(_fftSize, false, 0, 0);
}

AudioProcessor::~AudioProcessor()
{
    free(_cfg);
}

void
AudioProcessor::getSpectrogram(AudioDataAccessor& audioData, float* outputSpectrogram)
{
    const float mean = getMeanValue(audioData, _audioLength);
    const float maxDeviation = getMaxDeviationValue(audioData, _audioLength, mean);

    assert(outputSpectrogram != nullptr);

    const auto startIndex = audioData.pos();
    for (std::size_t windowStart = startIndex;
         windowStart < startIndex + _audioLength - _windowSize;
         windowStart += _stepSize) {
        audioData.seek(windowStart);
        for (int i = 0; i < _windowSize; i++) {
            _fftInput[i] = (static_cast<float>(audioData.next()) - mean) / maxDeviation;
        }
        for (int i = _windowSize; i < _fftSize; i++) {
            _fftInput[i] = 0;
        }
        getSpectrogramSegment(outputSpectrogram);
        outputSpectrogram += _pooledEnergySize;
    }
}

void
AudioProcessor::getSpectrogramSegment(float* outputSpectrogramRow)
{
    _hammingWindow.applyWindow(_fftInput.get());

    kiss_fftr(_cfg, _fftInput.get(), _fftOutput.get());

    for (int i = 0; i < _energySize; i++) {
        const float real = _fftOutput[i].r;
        const float imag = _fftOutput[i].i;
        const float magSquared = (real * real) + (imag * imag);
        _energy[i] = magSquared;
    }

    float* outputSrc = _energy.get();
    float* outputDst = outputSpectrogramRow;
    for (int i = 0; i < _energySize; i += _poolingSize) {
        float average = 0;
        for (int j = 0; j < _poolingSize; j++) {
            if (i + j < _energySize) {
                average += *outputSrc;
                outputSrc++;
            }
        }
        *outputDst = average / _poolingSize;
        outputDst++;
    }

    for (int i = 0; i < _pooledEnergySize; i++) {
        outputSpectrogramRow[i] = log10f(outputSpectrogramRow[i] + FLT_EPSILON);
    }
}