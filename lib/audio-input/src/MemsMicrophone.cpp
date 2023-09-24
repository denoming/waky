#include "audio-input/MemsMicrophone.hpp"

#include "audio-input/MemoryPool.hpp"

#include <esp_log.h>
#include <esp_assert.h>

#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

static const char* TAG = "ESP32 TFLITE WWD - MM";

// I2S interface pins configuration
#define WAKY_I2S_MIC_SCK CONFIG_WAKY_I2S_MIC_SCK
#define WAKY_I2S_MIC_WS CONFIG_WAKY_I2S_MIC_WS
#define WAKY_I2S_MIC_SD CONFIG_WAKY_I2S_MIC_SD
// I2S audio configuration
#define WAKY_SAMPLE_RATE (16000) // The total amount of samples per second
#define WAKY_SAMPLE_BITS (I2S_DATA_BIT_WIDTH_32BIT) // The lngth of each sample in bits
#define WAKY_SAMPLE_MODE (I2S_SLOT_MODE_MONO) // Mono/Stereo

/* I2S interface configuration */
const i2s_std_config_t I2S_CONFIG = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(WAKY_SAMPLE_RATE),
    /***
     * INMP1441:
     *  - 64 SCK cycles in each WS stereo frame (or 32 SCK cycles per data-word)
     *  - 24bit per channel
     *  - MSB first with one SCK cycle delay
     ***/
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(WAKY_SAMPLE_BITS, WAKY_SAMPLE_MODE),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = gpio_num_t(WAKY_I2S_MIC_SCK),
        .ws = gpio_num_t(WAKY_I2S_MIC_WS),
        .dout = I2S_GPIO_UNUSED,
        .din = gpio_num_t(WAKY_I2S_MIC_SD),
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
    },
};

MemsMicrophone::MemsMicrophone(MemoryPool& memoryPool)
    : _channelHandle{}
    , _accessor{memoryPool}
    , _waiter{nullptr}
{
}

bool
MemsMicrophone::start(TaskHandle_t waiter)
{
    static const int32_t kTaskStackDepth = 4096u;
    static const int32_t kTaskPinnedCore = 1;

    i2s_chan_config_t cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    if (i2s_new_channel(&cfg, nullptr, &_channelHandle) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to crease audio channel");
        return false;
    }

    if (i2s_channel_init_std_mode(_channelHandle, &I2S_CONFIG) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to initialize audio channel");
        return false;
    }

    if (i2s_channel_enable(_channelHandle) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to enable audio channel");
        return false;
    }

    _waiter = waiter;
    if (xTaskCreatePinnedToCore(&MemsMicrophone::pullAudioDataTask,
                                "PULL_DATA",
                                kTaskStackDepth,
                                this,
                                tskIDLE_PRIORITY,
                                nullptr,
                                kTaskPinnedCore)
        != pdPASS) {
        _waiter = nullptr;
        ESP_LOGE(TAG, "Unable to create pull data task");
        return false;
    }

    return true;
}

AudioDataAccessor
MemsMicrophone::data()
{
    return _accessor.clone();
}

std::size_t
MemsMicrophone::pullAudioData(uint8_t* buffer, size_t size)
{
    size_t bytesRead{0};
    ESP_ERROR_CHECK(i2s_channel_read(_channelHandle, buffer, size, &bytesRead, portMAX_DELAY));
    return bytesRead;
}

void
MemsMicrophone::processAudioData(const uint8_t* buffer, std::size_t size)
{
    static const int kDataBitShift = 11;

    const auto* samples = reinterpret_cast<const int32_t*>(buffer);
    assert(samples != nullptr);
    for (int i = 0; i < size / sizeof(int32_t); ++i) {
        _accessor.put(static_cast<int16_t>(samples[i] >> kDataBitShift));
    }
}

[[noreturn]] void
MemsMicrophone::pullAudioDataTask(void* param)
{
    static const size_t kNotifyThreshold = 1600;
    static const size_t kBufferSize = 1024;

    assert(param != nullptr);
    auto* mic = static_cast<MemsMicrophone*>(param);

#ifdef DEBUG
    printStackInfo(TAG, "Before pulling of data");
#endif

    /**
     * Pulling audio data loop (PULL_DATA task: CPU1)
     */
    size_t totalBytes{0};
    uint8_t buffer[kBufferSize];
    while (true) {
        size_t bytesRead = mic->pullAudioData(buffer, kBufferSize);
        if (bytesRead > 0) {
            mic->processAudioData(buffer, bytesRead);
            totalBytes += bytesRead;
            if (totalBytes >= kNotifyThreshold) {
                totalBytes -= kNotifyThreshold;
                mic->notify();
            }
        }
    }
}

void
MemsMicrophone::notify()
{
    /* Send a direct to task notification  */
    xTaskNotify(_waiter, 1, eSetBits);
}