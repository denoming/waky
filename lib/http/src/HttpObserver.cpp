#include "http/HttpObserver.hpp"

#ifdef DEBUG
#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "WAKY[HttpObserver]";
#endif

void
HttpObserver::onError(int error)
{
#ifdef DEBUG
    ESP_LOGD(TAG, "OnError(%d)", error);
#endif
}

void
HttpObserver::onHeader(const char* key, const char* value)
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onHeader(%s, %s)", key, value);
#endif
}

void
HttpObserver::onHeaderSent()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onHeaderSent()");
#endif
}

void
HttpObserver::onData(void* data, int len)
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onData(%p, %d)", data, len);
#endif
}

void
HttpObserver::onFinish()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onFinish()");
#endif
}

void
HttpObserver::onConnect()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onConnect()");
#endif
}

void
HttpObserver::onDisconnect()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onDisconnect()");
#endif
}

void
HttpObserver::onRedirect()
{
#ifdef DEBUG
    ESP_LOGD(TAG, "onRedirect()");
#endif
}