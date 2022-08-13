#include "AudioDataAccessor.hpp"

#include <esp_assert.h>

#include "MemoryPool.hpp"

AudioDataAccessor::AudioDataAccessor(MemoryPool& memoryPool)
    : _memoryPool{memoryPool}
    , _index{0}
{
}

void
AudioDataAccessor::set(std::int16_t sample)
{
    assert(_index < MemoryPool::capacity());
    _memoryPool.set(_index, sample);
}

std::int16_t
AudioDataAccessor::get() const
{
    assert(_index < MemoryPool::capacity());
    return _memoryPool.get(_index);
}

void
AudioDataAccessor::put(std::int16_t sample)
{
    set(sample);
    next();
}

std::int16_t
AudioDataAccessor::next()
{
    std::int16_t sample = get();
    _index = (_index + 1) % MemoryPool::capacity();
    return sample;
}

int
AudioDataAccessor::pos() const
{
    return _index;
}

void
AudioDataAccessor::seek(int index)
{
    int totalSize = MemoryPool::capacity();
    _index = (index + totalSize) % totalSize;
}

AudioDataAccessor
AudioDataAccessor::clone()
{
    AudioDataAccessor copy{_memoryPool};
    copy.seek(pos());
    return copy;
}