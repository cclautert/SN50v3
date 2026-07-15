#ifndef __SHT40_H__
#define __SHT40_H__

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
/* Exported constants --------------------------------------------------------*/
#define SHT40_I2C_ADDR        0x44 // I2C address (SHT40-AD1B)
#define SHT40_CMD_MEAS_HIGH   0xFD // measure T & RH, high precision   (~8.2 ms)
#define SHT40_CMD_MEAS_MEDIUM 0xF6 // measure T & RH, medium precision (~4.5 ms)
#define SHT40_CMD_MEAS_LOW    0xE0 // measure T & RH, low precision    (~1.6 ms)
#define SHT40_CMD_READ_SERIAL 0x89 // read serial number
#define SHT40_CMD_SOFT_RESET  0x94 // soft reset

// Measurement precision (higher precision = longer measurement time / more power)
typedef enum{
  SHT40_PRECISION_HIGH,   // cmd 0xFD, ~8.2 ms
  SHT40_PRECISION_MEDIUM, // cmd 0xF6, ~4.5 ms
  SHT40_PRECISION_LOW,    // cmd 0xE0, ~1.6 ms
} sht40_precision_t;
/* Exported functions ------------------------------------------------------- */

//=============================================================================
// Calculates/checks the Sensirion CRC8 (poly 0x131, init 0xFF) over the two
// data bytes at Result[offset] and compares it with Result[offset+2].
// return: 1 = checksum ok, 0 = mismatch
//-----------------------------------------------------------------------------
uint8_t SHT40_CheckSum_CRC8(uint8_t* Result, uint8_t offset);

//=============================================================================
// Detects an SHT40 on the I2C bus by reading its serial number (cmd 0x89) and
// validating the CRC. The serial command is not understood by an SHT3x, so a
// valid CRC distinguishes an SHT40 from an SHT3x sharing address 0x44.
// return: 1 = SHT40 present, 0 = not present
//-----------------------------------------------------------------------------
uint8_t check_sht40_connect(void);

//=============================================================================
// Reads temperature [degC] and relative humidity [%RH] from the SHT40 into the
// sht3x_data_t struct (temp_sht / hum_sht) using the given measurement
// precision. On persistent failure it writes the sentinel values
// temp_sht=3276.7, hum_sht=6553.5 (same convention as SHT31_Read).
//-----------------------------------------------------------------------------
// input: precision  SHT40_PRECISION_HIGH / _MEDIUM / _LOW
//-----------------------------------------------------------------------------
void SHT40_Read(sht3x_data_t* sht40_data, sht40_precision_t precision);

#ifdef __cplusplus
}
#endif

#endif /* __SHT40_H__ */
