#include "wifi/WiFi.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_check.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include <nvs_flash.h>

#include "wifi/Config.hpp"

static const char* TAG = "WAKY[WiFi]";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

class WiFiImpl {
public:
    WiFiImpl() = default;

    bool
    setup();

    bool
    connect(uint32_t timeout);

    void
    finalise();

private:
    void
    onWiFiEvent(int32_t id, void* data);

    void
    onIpEvent(int32_t id, void* data);

    static void
    eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

private:
    EventGroupHandle_t _eventGroup{};
    esp_event_handler_instance_t _wifiEventHandler{};
    esp_event_handler_instance_t _ipEventHandler{};
    esp_netif_t* _netInter{};
    std::size_t _retryCnt{};
};

bool
WiFiImpl::setup()
{
    _eventGroup = xEventGroupCreate();
    ESP_RETURN_ON_FALSE(_eventGroup != nullptr, false, TAG, "Unable to create event group");

    esp_err_t rv = nvs_flash_init();
    if (rv == ESP_ERR_NVS_NO_FREE_PAGES || rv == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        rv = nvs_flash_init();
    }
    if (rv != ESP_OK) {
        ESP_LOGE(TAG, "Unable to initialise NVS flash memory");
        return false;
    }

    if (esp_netif_init() != ESP_OK) {
        ESP_LOGE(TAG, "Unable to initialise network interface");
        return false;
    }

    if (esp_event_loop_create_default() != ESP_OK) {
        ESP_LOGE(TAG, "Unable to create event loop");
        return false;
    }

    _netInter = esp_netif_create_default_wifi_sta();
    assert(_netInter != nullptr);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to initialise WiFi");
        esp_netif_destroy_default_wifi(_netInter);
        return false;
    }

    if (esp_event_handler_instance_register(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, this, &_wifiEventHandler)
        != ESP_OK) {
        ESP_LOGE(TAG, "Unable to register event handler (WiFi)");
        return false;
    }
    if (esp_event_handler_instance_register(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandler, this, &_ipEventHandler)
        != ESP_OK) {
        ESP_LOGE(TAG, "Unable to register event handler (IP)");
        return false;
    }

    wifi_config_t wifiConfig = {
        .sta = {
            .ssid = WAKY_WIFI_SSID,
            .password = WAKY_WIFI_PASSWORD,
            .threshold = {
                .authmode = WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD
            },
            .sae_pwe_h2e = WAKY_WIFI_SAE_MODE,
            .sae_h2e_identifier = WAKY_WIFI_H2E_IDENTIFIER,
        },
    };

    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set WiFi mode");
        return false;
    }

    if (esp_wifi_set_config(WIFI_IF_STA, &wifiConfig) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set WiFi config");
        return false;
    }

    return true;
}

bool
WiFiImpl::connect(uint32_t timeout)
{
    if (esp_wifi_start() != ESP_OK) {
        ESP_LOGE(TAG, "Unable to start WiFi in STA mode");
        return false;
    }
#ifdef DEBUG
    ESP_LOGD(TAG, "Connecting to AP SSID: %s", WAKY_WIFI_SSID);
#endif

    /**
     * Waiting until
     * - the connection is established (WIFI_CONNECTED_BIT)
     * - connection failed for the maximum number of re-tries (WIFI_FAIL_BIT)
     */
    const EventBits_t bits = xEventGroupWaitBits(
        _eventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, timeout);
    if (bits & WIFI_CONNECTED_BIT) {
#ifdef DEBUG
        ESP_LOGD(TAG, "Connecting to AP SSID was successful: %s", WAKY_WIFI_SSID);
#endif
        return true;
    }
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Unable to connect to SSID: %s", WAKY_WIFI_SSID);
        return true;
    }

    ESP_LOGE(TAG, "Unexpected event");
    return false;
}

void
WiFiImpl::finalise()
{
    _retryCnt = 0;

    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, _wifiEventHandler);
    _wifiEventHandler = {};
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, _ipEventHandler);
    _ipEventHandler = {};

    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();

    esp_netif_destroy_default_wifi(_netInter);
    _netInter = {};

    esp_event_loop_delete_default();

    vEventGroupDelete(_eventGroup);
    _eventGroup = {};
}

void
WiFiImpl::onWiFiEvent(int32_t id, void* /*eventData*/)
{
    switch (id) {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        if (_retryCnt < WAKY_MAXIMUM_RETRY) {
            esp_wifi_connect();
            _retryCnt++;
#ifdef DEBUG
            ESP_LOGD(TAG, "Retry to connect to the AP");
#endif
        } else {
            xEventGroupSetBits(_eventGroup, WIFI_FAIL_BIT);
        }
        break;
    default:
        break;
    }
}

void
WiFiImpl::onIpEvent(int32_t id, void* eventData)
{
    if (id == IP_EVENT_STA_GOT_IP) {
        auto* event = static_cast<ip_event_got_ip_t*>(eventData);
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        _retryCnt = 0;
        xEventGroupSetBits(_eventGroup, WIFI_CONNECTED_BIT);
    }
}

void
WiFiImpl::eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    auto* const self = static_cast<WiFiImpl*>(arg);
    assert(self != nullptr);

    if (eventBase == WIFI_EVENT) {
        self->onWiFiEvent(eventId, eventData);
    } else if (eventBase == IP_EVENT) {
        self->onIpEvent(eventId, eventData);
    }

    /* Not interesting events */
}

WiFi::WiFi()
{
    static WiFiImpl impl;
    _impl = &impl;
}

WiFi::~WiFi()
{
    _impl = nullptr;
}

bool
WiFi::setup()
{
    assert(_impl != nullptr);
    return _impl->setup();
}

bool
WiFi::connect(uint32_t timeout)
{
    assert(_impl != nullptr);
    return _impl->connect(timeout);
}

void
WiFi::finalise()
{
    assert(_impl != nullptr);
    return _impl->finalise();
}