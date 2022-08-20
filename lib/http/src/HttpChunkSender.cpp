#include "http/HttpChunkSender.hpp"

#include "http/HttpClient.hpp"

HttpChunkSender::HttpChunkSender(HttpClient& client)
    : _client{client}
{
}

bool
HttpChunkSender::send(const char* data, std::size_t size)
{
    assert(data != nullptr);
    assert(size > 0);
    bool status = startChunk(size);
    if (status) {
        status = writeChunk(data, size);
    }
    if (status) {
        status = endChunk();
    }
    return status;
}

bool
HttpChunkSender::send(const char* data)
{
    return send(data, strlen(data));
}

bool
HttpChunkSender::startChunk(std::size_t size)
{
    assert(size > 0);
    return (_client.printf("%X\r\n", size) > 0);
}

bool
HttpChunkSender::writeChunk(const char* data, std::size_t size)
{
    assert(data != nullptr);
    assert(size > 0);
    return (_client.write(data, size) == size);
}

bool
HttpChunkSender::endChunk()
{
    return (_client.print("\r\n") > 0);
}

bool
HttpChunkSender::finalize()
{
    return (_client.print("0\r\n\r\n") > 0);
}