#include "ze07.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Frame constants (datasheet Table 4 - 8)
 * ---------------------------------------------------------------------- */
#define ZE07_FRAME_LEN      9u
#define ZE07_START_BYTE     0xFFu
#define ZE07_GAS_TYPE_CO    0x04u
#define ZE07_UNIT_PPM       0x03u
#define ZE07_QAA_RESP_CMD   0x86u  /* echoed command byte in QAA response frame */

/* Command frames (9 bytes each) */
static const uint8_t CMD_SWITCH_QAA[] = { 0xFF,0x01,0x78,0x41,0x00,0x00,0x00,0x00,0x46 };
static const uint8_t CMD_SWITCH_IU[]  = { 0xFF,0x01,0x78,0x40,0x00,0x00,0x00,0x00,0x47 };
static const uint8_t CMD_QA_QUERY[]   = { 0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79 };

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/** @brief Compute two's-complement checksum over bytes [1..7] of a 9-byte frame. */
static uint8_t _ze07_checksum(const uint8_t *frame)
{
    uint8_t sum = 0;
    for (int i = 1; i <= 7; i++) {
        sum += frame[i];
    }
    return (uint8_t)((~sum) + 1u);
}

/**
 * @brief Parse a validated 9-byte frame into ppm.
 *        IU mode  : concentration = (byte4 * 256 + byte5) * 0.1
 *        QAA mode : concentration = (byte2 * 256 + byte3) * 0.1
 */
static float _ze07_parse_ppm(const uint8_t *frame, ZE07_Mode mode)
{
    uint16_t raw = (mode == ZE07_MODE_QAA) ? (((uint16_t)frame[2] << 8) | frame[3]) : (((uint16_t)frame[4] << 8) | frame[5]);
    return raw * 0.1f;
}

/** @brief Read and validate one 9-byte frame from the sensor */
static ZE07_Status _ze07_read_frame(ZE07_Dev *dev, uint8_t *frame)
{
    uint8_t b;
    int tries = 0;

    do {
        if (dev->hal.uart_read(&b, 1) != 0) {
            return ZE07_ERROR_UART;
        }
        if (++tries > ZE07_FRAME_LEN * 3) {
            return ZE07_ERROR_FRAME;
        }
    } while (b != ZE07_START_BYTE);

    frame[0] = b;

    if (dev->hal.uart_read(&frame[1], ZE07_FRAME_LEN - 1) != 0) {
        return ZE07_ERROR_UART;
    }

    if (dev->mode == ZE07_MODE_QAA) {
        if (frame[1] != ZE07_QAA_RESP_CMD)  return ZE07_ERROR_FRAME;
    } else {
        if (frame[1] != ZE07_GAS_TYPE_CO)   return ZE07_ERROR_FRAME;
        if (frame[2] != ZE07_UNIT_PPM)      return ZE07_ERROR_FRAME;
    }

    if (frame[8] != _ze07_checksum(frame))  return ZE07_ERROR_FRAME;

    return ZE07_OK;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

ZE07_Status ZE07_Init(ZE07_Dev *dev)
{
    dev->mode = ZE07_MODE_IU;
    return ZE07_OK;
}

ZE07_Status ZE07_SetMode(ZE07_Dev *dev, ZE07_Mode mode)
{
    const uint8_t *cmd = (mode == ZE07_MODE_QAA) ? CMD_SWITCH_QAA : CMD_SWITCH_IU;

    if (dev->hal.uart_write(cmd, ZE07_FRAME_LEN) != 0) {
        return ZE07_ERROR_UART;
    }

    dev->mode = mode;
    return ZE07_OK;
}

ZE07_Status ZE07_Read(ZE07_Dev *dev, float *ppm)
{
    uint8_t frame[ZE07_FRAME_LEN];
    ZE07_Status s;

    if (dev->mode == ZE07_MODE_QAA) {
        if (dev->hal.uart_write(CMD_QA_QUERY, ZE07_FRAME_LEN) != 0) {
            return ZE07_ERROR_UART;
        }
    }

    s = _ze07_read_frame(dev, frame);
    if (s != ZE07_OK) {
        return s;
    }

    *ppm = _ze07_parse_ppm(frame, dev->mode);
    return ZE07_OK;
}