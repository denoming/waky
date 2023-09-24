#pragma once

#include "sdkconfig.h"

#define WAKY_WIFI_SSID CONFIG_WAKY_WIFI_SSID
#define WAKY_WIFI_PASSWORD CONFIG_WAKY_WIFI_PASSWORD
#define WAKY_MAXIMUM_RETRY CONFIG_WAKY_MAXIMUM_RETRY

#if CONFIG_WAKY_WIFI_WPA3_SAE_PWE_HUNT_AND_PECK
#define WAKY_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define WAKY_WIFI_H2E_IDENTIFIER ""
#elif CONFIG_WAKY_WIFI_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define WAKY_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define WAKY_WIFI_H2E_IDENTIFIER CONFIG_WAKY_WIFI_PW_ID
#elif CONFIG_WAKY_WIFI_WPA3_SAE_PWE_BOTH
#define WAKY_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define WAKY_WIFI_H2E_IDENTIFIER CONFIG_WAKY_WIFI_PW_ID
#endif

#if CONFIG_WAKY_WIFI_AUTH_OPEN
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_WAKY_WIFI_AUTH_WEP
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_WAKY_WIFI_AUTH_WPA_PSK
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_WAKY_WIFI_AUTH_WPA2_PSK
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_WAKY_WIFI_AUTH_WPA_WPA2_PSK
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_WAKY_WIFI_AUTH_WPA3_PSK
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_WAKY_WIFI_AUTH_WPA2_WPA3_PSK
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_WAKY_WIFI_AUTH_WAPI_PSK
#define WAKY_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif