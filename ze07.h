#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * UART settings
 * ---------------------------------------------------------------------- */
#define ZE07_CFG_UART_BAUD_RATE     9600    /* UART Baudrate */
#define ZE07_CFG_UART_DATA_BITS     8       /* UART data bits */
#define ZE07_CFG_UART_STOP_BITS     1       /* UART stop bits */
#define ZE07_CFG_UART_PARITY        0       /* UART partity (None) */

/* -------------------------------------------------------------------------
 * Sensor characteristics
 * ---------------------------------------------------------------------- */
#define ZE07_WARMUP_MS              180000  /* 3 min warm-up before valid readings */
#define ZE07_RESPONSE_TIME_MS       60000   /* max time to detect gas change */
#define ZE07_RESUME_TIME_MS         60000   /* max time to recover after gas removed */
#define ZE07_STREAM_INTERVAL_MS     1000    /* initiative mode: 1 frame/s */
#define ZE07_RANGE_PPM              500     /* 0 ~ 500 ppm */
#define ZE07_RESOLUTION_PPM         0.1f    /* 0.1 ppm */

/** @brief ZE07 status code */
typedef enum {
    ZE07_OK            =  0,
    ZE07_ERROR_UART    = -1,  /* uart_read / uart_write failed */
    ZE07_ERROR_FRAME   = -2,  /* bad start byte or checksum mismatch */
} ZE07_Status;

/** @brief Communication mode */
typedef enum {
    ZE07_MODE_IU  = 0,  /* Initiative upload mode - auto streams 1 frame/s (default) */
    ZE07_MODE_QAA = 1,  /* Question and Answer mode */
} ZE07_Mode;

/** @brief HAL callbacks — caller fills these in before calling ZE07_Init() */
typedef struct {
    int (*uart_read) (uint8_t *buf, size_t len);        /* 0 on success, negative on error */
    int (*uart_write)(const uint8_t *buf, size_t len);  /* 0 on success, negative on error */
} ZE07_Hal;

/** @brief Device handle */
typedef struct {
    ZE07_Hal  hal;
    ZE07_Mode mode;
} ZE07_Dev;

/**
 * @brief Initialize device, defaults to IU mode.
 * @param dev pointer to device handle.
 * @note Read one frame to confirm sensors presence.
 * @return ZE07_OK on success, others on fail.
 */
ZE07_Status ZE07_Init(ZE07_Dev *dev);

/**
 * @brief Switch communication mode and send corresponding command frame.
 * @param dev  pointer to device handle.
 * @param mode ZE07_MODE_IU or ZE07_MODE_QAA.
 * @return ZE07_OK on success, others on fail.
 */
ZE07_Status ZE07_SetMode(ZE07_Dev *dev, ZE07_Mode mode);

/**
 * @brief Read one CO concentration sample.
 *        IU mode : reads next streamed frame.
 *        QAA mode: sends query then reads response.
 * @param dev      pointer to device handle.
 * @param[out] ppm CO concentration in ppm.
 * @return ZE07_OK on success, others on fail.
 */
ZE07_Status ZE07_Read(ZE07_Dev *dev, float *ppm);

#ifdef __cplusplus
}
#endif