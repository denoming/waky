#include "audio-input/MemsMicrophone.hpp"

#include "audio-input/MemoryPool.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_assert.h>

#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

static const char* TAG = "ESP32 TFLITE WWD - MM";

static const i2s_config_t I2S_CONFIG = {
    .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = CONFIG_JRVA_I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = CONFIG_JRVA_I2S_DMA_BUFFER_COUNT,
    .dma_buf_len = CONFIG_JRVA_I2S_DMA_BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
    .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT,
};

static const i2s_pin_config_t I2S_PIN_CONFIG = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = CONFIG_JRVA_I2S_MIC_SCK,
    .ws_io_num = CONFIG_JRVA_I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = CONFIG_JRVA_I2S_MIC_SD,
};

MemsMicrophone::MemsMicrophone(MemoryPool& memoryPool)
    : MemsMicrophone{
        I2S_PIN_CONFIG, static_cast<i2s_port_t>(CONFIG_JRVA_I2S_MIC_PORT), I2S_CONFIG, memoryPool}
{
}

MemsMicrophone::MemsMicrophone(i2s_pin_config_t pins,
                               i2s_port_t port,
                               i2s_config_t config,
                               MemoryPool& memoryPool)
    : _pins{pins}
    , _port{port}
    , _config{config}
    , _accessor{memoryPool}
    , _waiter{nullptr}
{
}

bool
MemsMicrophone::start(TaskHandle_t waiter)
{
    static const int kQueueSize = 8;
    static const uint32_t kTaskStackDepth = 2048u;
    static const UBaseType_t kTaskPriority
        = UBaseType_t((tskIDLE_PRIORITY + 1) | portPRIVILEGE_BIT);

    if (i2s_driver_install(_port, &_config, kQueueSize, &_queue) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2S driver");
        return false;
    }

    if (i2s_set_pin(_port, &_pins) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set I2S pin");
        return false;
    }

    _waiter = waiter;
    const auto rv = xTaskCreate(&MemsMicrophone::pullDataTask,
                                "MEMS microphone pull data task",
                                kTaskStackDepth,
                                this,
                                kTaskPriority,
                                nullptr);
    if (rv != pdPASS) {
        _waiter = nullptr;
        ESP_LOGE(TAG, "Failed to create pull data task");
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
MemsMicrophone::pullData(uint8_t* buffer, std::size_t size)
{
    static const TickType_t kTimeout = 100 / portTICK_PERIOD_MS;

    std::size_t bytesRead{0};
    i2s_event_t event;
    if (xQueueReceive(_queue, &event, portMAX_DELAY) == pdPASS) {
        if (event.type == I2S_EVENT_RX_DONE) {
            ESP_ERROR_CHECK(i2s_read(_port, buffer, size, &bytesRead, kTimeout));
        }
    }
    return bytesRead;
}

void
MemsMicrophone::processData(const uint8_t* buffer, std::size_t size)
{
    static const int kDataBitShift = 11;

    const auto* samples = reinterpret_cast<const int32_t*>(buffer);
    assert(samples != nullptr);
    for (int i = 0; i < size / sizeof(int32_t); ++i) {
        _accessor.put(samples[i] >> kDataBitShift);
    }
}

void
MemsMicrophone::pullDataTask(void* param)
{
    static const std::size_t kNotifyThreshold = 1600;
    static const std::size_t kBufferSize = CONFIG_JRVA_I2S_DMA_BUFFER_COUNT
                                           * CONFIG_JRVA_I2S_DMA_BUFFER_SIZE
                                           * CONFIG_JRVA_I2S_SAMPLE_BYTES;

    assert(param != nullptr);
    MemsMicrophone* mic = static_cast<MemsMicrophone*>(param);

    static uint8_t buffer[kBufferSize];
#ifdef DEBUG
    printStackInfo(TAG, "Before pulling of data");
#endif
    std::size_t totalBytes{0};
    while (true) {
        std::size_t bytesRead{0};
        bytesRead = mic->pullData(buffer, kBufferSize);
        if (bytesRead > 0) {
            mic->processData(buffer, bytesRead);
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
    xTaskNotify(_waiter, 1, eSetBits);
}