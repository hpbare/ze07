# ZE07

## Overview

ZE07-CO is a UART-based electrochemical carbon monoxide (CO) sensor. It supports two communication modes:

- `Initiative Upload (IU)` - sensor automatically streams 1 frame/s (default mode).
- `Question and Answer (QAA)` - host sends a query command, sensor replies once.

### Technical Parameters

| Parameter             | Value                                         |
| --------------------- | --------------------------------------------- |
| Target gas            | CO                                            |
| Interference gas      | Alcohol and other gases                       |
| Detection range       | 0~500 ppm                                     |
| Resolution            | 0.1 ppm                                       |
| Output                | UART (3V-TTL) / DAC (0.4~2V, 0~full scale)    |
| Working voltage       | 5V~12V (no reverse-polarity protection)       |
| Warm-up time          | ≤3 min                                        |
| Response time         | ≤60 s                                         |
| Recovery time         | ≤60 s                                         |
| Operating temperature | -10°C~55°C                                    |
| Operating humidity    | 15%RH~90%RH (no condensation)                 |
| Working life          | 3-5 years (in air)                            |

### Pin Description

| Pin  | Function                           |
| ---- | ---------------------------------- |
| 1    | Reserved                           |
| 2    | NC                                 |
| 3    | Reserved                           |
| 4    | Reserved                           |
| 5    | GND                                |
| 6    | NC                                 |
| 7    | UART RXD (0~3.0V, data input)      |
| 8    | UART TXD (0~3.0V, data output)     |
| 9    | Reserved                           |
| 10   | DAC output (0.4V~2V, 0~full scale) |
| 11   | NC                                 |
| 12   | NC                                 |
| 13   | NC                                 |
| 14   | GND                                |
| 15   | Vin (5V~12V)                       |

## Communication Protocol

### UART settings

| Configuration  | Value |
| -------------- | ----- |
| Baudrate       | 9600  |
| Data bits      | 8     |
| Stop bits      | 1     |
| Parity         | None  |

### UART command

All frames (commands and responses) are **9 bytes** long.

#### Switch to QAA mode

| Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 |
| ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| 0xFF   | 0x01   | 0x78   | 0x41   | 0x00   | 0x00   | 0x00   | 0x00   | 0x46   |

#### Switch to IU mode

| Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 |
| ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| 0xFF   | 0x01   | 0x78   | 0x40   | 0x00   | 0x00   | 0x00   | 0x00   | 0x47   |

#### QAA query command

Host sends this frame to request one reading while in QAA mode:

| Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 |
| ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| 0xFF   | 0x01   | 0x86   | 0x00   | 0x00   | 0x00   | 0x00   | 0x00   | 0x79   |

### Response frame

#### IU mode (auto-streamed, 1 frame/s)

| Byte | Meaning                    |
| ---- | -------------------------- |
| 0    | Start byte (0xFF)          |
| 1    | Gas type (0x04 = CO)       |
| 2    | Unit (0x03 = ppm)          |
| 3    | No. of decimal places      |
| 4    | Concentration high byte    |
| 5    | Concentration low byte     |
| 6    | Full range high byte       |
| 7    | Full range low byte        |
| 8    | Checksum                   |

Concentration (ppm) = `(byte[4] << 8 | byte[5]) * 0.1`

#### QAA mode (response to query command)

| Byte | Meaning                      |
| ---- | ---------------------------- |
| 0    | Start byte (0xFF)            |
| 1    | Echoed command byte (0x86)   |
| 2    | Concentration high byte      |
| 3    | Concentration low byte       |
| 4-7  | Reserved / full-range data   |
| 8    | Checksum                     |

Concentration (ppm) = `(byte[2] << 8 | byte[3]) * 0.1`

### Checksum

Two's-complement sum over bytes 1 to 7:

```
checksum = ~(byte[1] + byte[2] + ... + byte[7]) + 1
```

Byte 8 of every frame (command or response) must equal this value.

## Cautions

- DO NOT insert or extract the sensor on the PCB board.
- DO NOT change or move the electronic part on the module.
- Avoid sensor contact with organic solvent, coatings, medicine, oil and high concentration gases.
- Excessive impact or vibration should be avoided.
- Please keep the modules warming up for at least 5 minutes when first using.
- Please do not use the modules in systems which related to human being’s safety.
- Please do not use the modules in strong air convection environment.
- Please do not expose the modules in high concentration organic gas for a long time.