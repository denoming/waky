#include "audio-input/AudioDataAccessor.hpp"

#include <esp_assert.h>

#include "audio-input/MemoryPool.hpp"

AudioDataAccessor::AudioDataAccessor(MemoryPool& memoryPool)
    : _memoryPool{memoryPool}
    , _index{0}
{
}

void
AudioDataAccessor::set(int16_t sample)
{
    assert(_index < MemoryPool::capacity());
    _memoryPool.set(_index, sample);
}

int16_t
AudioDataAccessor::get() const
{
    assert(_index < MemoryPool::capacity());
    return _memoryPool.get(_index);
}

void
AudioDataAccessor::put(int16_t sample)
{
    set(sample);
    next();
}

int16_t
AudioDataAccessor::next()
{
    int16_t sample = get();
    _index = (_index + 1) % MemoryPool::capacity();
    return sample;
}

size_t
AudioDataAccessor::pos() const
{
    return _index;
}

void
AudioDataAccessor::seek(long offset)
{
    /* Normalization of negative offset */
    _index = (offset + MemoryPool::capacity()) % MemoryPool::capacity();
}

AudioDataAccessor
AudioDataAccessor::clone()
{
    AudioDataAccessor copy{_memoryPool};
    copy.seek(pos());
    return copy;
}