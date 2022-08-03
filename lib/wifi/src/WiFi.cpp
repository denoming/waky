#include "WiFi.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_check.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include <nvs_flash.h>

#include <string.h>

static const char* TAG = "ESP32 JRVA - WiFi";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WIFI_SSID CONFIG_JRVA_WIFI_SSID
#define WIFI_PASS CONFIG_JRVA_WIFI_PASSWORD
#define WIFI_MAXIMUM_RETRY CONFIG_JRVA_WIFI_MAXIMUM_RETRY

namespace {

void
initializeFlash()
{
    auto error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        error = nvs_flash_init();
    }
    ESP_ERROR_CHECK(error);
}

} // namespace

class WiFi::Impl {
public:
    Impl();

    void
    initialize();

    bool
    setUp(uint32_t timeout);

    void
    tearDown();

private:
    void
    onWiFiEvent(int32_t id, void* data);

    void
    onIpEvent(int32_t id, void* data);

    static void
    eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

private:
    esp_netif_t* _netInter;
    EventGroupHandle_t _wifiEventGroup;
    esp_event_handler_instance_t _wifiEvent;
    esp_event_handler_instance_t _ipEvent;
    int _retryNumber;
};

WiFi::Impl::Impl()
    : _netInter{nullptr}
    , _wifiEventGroup{nullptr}
    , _wifiEvent{nullptr}
    , _ipEvent{nullptr}
    , _retryNumber{0}
{
}

void
WiFi::Impl::initialize()
{
    ESP_LOGD(TAG, "Initialize NVS flash");
    initializeFlash();

    ESP_LOGD(TAG, "Initialize TCP/IP stack");
    ESP_ERROR_CHECK(esp_netif_init());
}

bool
WiFi::Impl::setUp(uint32_t timeout)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    _wifiEventGroup = xEventGroupCreate();
    ESP_RETURN_ON_FALSE(_wifiEventGroup != NULL, false, TAG, "Failed to create event group");

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &Impl::eventHandler, this, &_wifiEvent));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &Impl::eventHandler, this, &_ipEvent));

    ESP_LOGD(TAG, "Create default WiFi STA");
    _netInter = esp_netif_create_default_wifi_sta();

    ESP_LOGD(TAG, "Initialize WiFi");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifiConfig = {};
    strcpy(reinterpret_cast<char*>(wifiConfig.sta.ssid), WIFI_SSID);
    strcpy(reinterpret_cast<char*>(wifiConfig.sta.password), WIFI_PASS);
    wifiConfig.sta.threshold = {
        .rssi = 0,
        .authmode = WIFI_AUTH_WPA2_PSK,
    };

    ESP_LOGD(TAG, "Starting WiFi");
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGD(TAG, "Waiting for connection to <%s> AP", WIFI_SSID);
    EventBits_t bits{WIFI_CONNECTED_BIT | WIFI_FAIL_BIT};
    bits = xEventGroupWaitBits(_wifiEventGroup, bits, pdFALSE, pdFALSE, timeout);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to <%s> AP", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed connecting to <%s> AP", WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "Unexpected event");
    }

    return true;
}

void
WiFi::Impl::tearDown()
{
    _retryNumber = 0;

    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, _wifiEvent);
    _wifiEvent = nullptr;
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, _ipEvent);
    _ipEvent = nullptr;

    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    esp_netif_destroy_default_wifi(_netInter);
    _netInter = nullptr;

    ESP_ERROR_CHECK(esp_event_loop_delete_default());

    vEventGroupDelete(_wifiEventGroup);
    _wifiEventGroup = nullptr;
}

void
WiFi::Impl::onWiFiEvent(int32_t id, void* data)
{
    switch (id) {
    case WIFI_EVENT_STA_START:
        ESP_LOGD(TAG, "Connectin to <%s> AP", WIFI_SSID);
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        if (_retryNumber < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            _retryNumber++;
            ESP_LOGD(TAG, "Retry connecting to <%s> AP", WIFI_SSID);
        } else {
            xEventGroupSetBits(_wifiEventGroup, WIFI_FAIL_BIT);
        }
        break;
    }
}

void
WiFi::Impl::onIpEvent(int32_t id, void* data)
{
    if (id == IP_EVENT_STA_GOT_IP) {
        auto* event = reinterpret_cast<ip_event_got_ip_t*>(data);
        ESP_LOGD(TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        _retryNumber = 0;
        xEventGroupSetBits(_wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

void
WiFi::Impl::eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    Impl* self = static_cast<Impl*>(arg);
    assert(self != nullptr);

    if (eventBase == WIFI_EVENT) {
        self->onWiFiEvent(eventId, eventData);
        return;
    }
    if (eventBase == IP_EVENT) {
        self->onIpEvent(eventId, eventData);
        return;
    }

    ESP_LOGE(TAG, "Unexpected event");
}

WiFi&
WiFi::create()
{
    static std::unique_ptr<WiFi> instance;
    if (!instance) {
        std::unique_ptr<WiFi::Impl> impl{new WiFi::Impl};
        impl->initialize();
        instance.reset(new WiFi{std::move(impl)});
    }
    return *instance;
}

WiFi::WiFi(std::unique_ptr<Impl> impl)
    : _impl{std::move(impl)}
{
}

bool
WiFi::setUp(TickType_t timeout)
{
    assert(_impl);
    return _impl->setUp(timeout);
}

void
WiFi::tearDown()
{
    assert(_impl);
    _impl->tearDown();
}
