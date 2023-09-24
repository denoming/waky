#pragma once

class HttpObserver {
public:
    virtual ~HttpObserver() = default;

    virtual void
    onError(int error);

    virtual void
    onHeader(const char* key, const char* value);

    virtual void
    onHeaderSent();

    virtual void
    onData(void* data, int len);

    virtual void
    onFinish();

    virtual void
    onConnect();

    virtual void
    onDisconnect();

    virtual void
    onRedirect();
};