#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ze07.h"

/* -------------------------------------------------------------------------
 * Example configuration - pick mode here
 * ---------------------------------------------------------------------- */
#define ZE07_EG_MODE                ZE07_MODE_IU
// #define ZE07_EG_MODE                ZE07_MODE_QAA
#define ZE07_EG_READ_INTERVAL_MS    1000
/* Uart configuration */
#define ZE07_UART_PORT          UART_NUM_2
#define ZE07_UART_TX_PIN        GPIO_NUM_17
#define ZE07_UART_RX_PIN        GPIO_NUM_16
#define ZE07_UART_BUF_SIZE      256
#define ZE07_UART_TIMEOUT_MS    2000

static const char *TAG = "[ZE07_EG]";

/* -------------------------------------------------------------------------
 * HAL implementation backed by ESP-IDF UART driver
 * ---------------------------------------------------------------------- */
static int hal_uart_read(uint8_t *buf, size_t len)
{
    int n = uart_read_bytes(ZE07_UART_PORT, buf, len, pdMS_TO_TICKS(ZE07_UART_TIMEOUT_MS));
    if (n != (int)len) {
        return -1;
    }
    return 0;
}

static int hal_uart_write(const uint8_t *buf, size_t len)
{
    int n = uart_write_bytes(ZE07_UART_PORT, (const char *)buf, len);
    if (n != (int)len) {
        return -1;
    }
    return 0;
}

static int uart_hw_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = ZE07_CFG_UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if(uart_driver_install(ZE07_UART_PORT, ZE07_UART_BUF_SIZE, ZE07_UART_BUF_SIZE, 0, NULL, 0) != ESP_OK){ return -1; }
    if(uart_param_config(ZE07_UART_PORT, &cfg) != ESP_OK){ return -1; }
    if(uart_set_pin(ZE07_UART_PORT, ZE07_UART_TX_PIN, ZE07_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK){ return -1; }

    return 0;
}

/* -------------------------------------------------------------------------
 * Example task
 * ---------------------------------------------------------------------- */
static void ze07_task(void *arg)
{
    (void)arg;

    ZE07_Dev dev = {
        .hal = {
            .uart_read  = hal_uart_read,
            .uart_write = hal_uart_write
        }
    };
    ZE07_Status st = ZE07_Init(&dev);
    if (st != ZE07_OK) {
        ESP_LOGE(TAG, "Failed to init ZE07: %d", st);
        vTaskDelete(NULL);
        return;
    }

    st = ZE07_SetMode(&dev, ZE07_EG_MODE);
    if (st != ZE07_OK) {
        ESP_LOGE(TAG, "Failed to set mode: %d", st);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Mode set to %s, warming up %d ms ...", (ZE07_EG_MODE == ZE07_MODE_QAA) ? "QAA" : "IU", ZE07_WARMUP_MS);
    vTaskDelay(pdMS_TO_TICKS(ZE07_WARMUP_MS));

    while (1) {
        float ppm = 0.0f;
        ZE07_Status r = ZE07_Read(&dev, &ppm);

        if (r == ZE07_OK) {
            ESP_LOGI(TAG, "CO = %.1f ppm", ppm);
        } else if (r == ZE07_ERROR_UART) {
            ESP_LOGW(TAG, "UART read/write error");
        } else if (r == ZE07_ERROR_FRAME) {
            ESP_LOGW(TAG, "Bad frame (start byte / checksum mismatch)");
        }
        vTaskDelay(pdMS_TO_TICKS(ZE07_EG_READ_INTERVAL_MS));
    }
}

void app_main(void)
{
    if(uart_hw_init() != 0){
        ESP_LOGE(TAG, "Failed to init UART.");
        return ;
    };
    xTaskCreate(ze07_task, "ze07_eg", 4096, NULL, 5, NULL);
}