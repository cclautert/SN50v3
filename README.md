Source Code for SN50v3-LB and SN50v3-NB.

SN50v3 uses LA66 module inside. See below for:

# How to Compile Firmware:
See Step 1 ~ Step 7 of [LA66 Compile Instruction](http://wiki.dragino.com/xwiki/bin/view/Main/User%20Manual%20for%20LoRaWAN%20End%20Nodes/LA66%20LoRaWAN%20Module/Compile%20and%20Upload%20Code%20to%20ASR6601%20Platform/ )

# How to Update Firmware:
## [via OTA: ](http://wiki.dragino.com/xwiki/bin/view/Main/Firmware%20OTA%20Update%20for%20Sensors/ )
## [via UART ](http://wiki.dragino.com/xwiki/bin/view/Main/UART%20Access%20for%20LoRa%20ST%20v4%20base%20model/ )

---

# SHT-40 sensor support (this fork)

This fork adds support for the Sensirion **SHT-40** temperature/humidity sensor,
in addition to the sensors already supported (SHT2x, SHT3x, BH1750, ...).

## How it works
- The SHT-40 (I2C address `0x44`) is **auto-detected** on the I2C bus at startup,
  in the temperature/humidity working modes **MOD 1, 3 and 12** — the same
  mechanism already used for SHT2x/SHT3x/BH1750.
- Because the SHT-40 shares address `0x44` with the SHT3x, it is identified by its
  serial-number command (`0x89` + CRC), which an SHT3x does not answer.
- Readings are written into the same `temp_sht` / `hum_sht` fields, so the
  **uplink payload and the server-side decoder are unchanged** — an SHT-40 reports
  exactly like an SHT3x.
- On boot the debug log prints `Use Sensor is SHT4x` when an SHT-40 is detected.

## Wiring
- SHT-40 on the I2C bus: **SCL = PA14**, **SDA = PA15**, 3.3 V, with pull-ups.

## Files
- `Drivers/sensor/sht40.c` / `sht40.h` — the SHT-40 driver.
  - Measurement precision: high (`0xFD`), medium (`0xF6`), low (`0xE0`).
  - Sensirion CRC-8 (poly `0x31`); conversions `T = -45 + 175*raw/65535`,
    `RH = -6 + 125*raw/65535` (clamped to 0..100).
  - Default precision is **high**. To change it, edit the
    `SHT40_Read(&data, SHT40_PRECISION_HIGH)` call in
    `Drivers/sensor/I2C_sensor.c` to `SHT40_PRECISION_MEDIUM` / `_LOW`.
- `Drivers/sensor/bsp.c` — `check_sht40_connect()` sets the detection flag in
  `BSP_sensor_Init()`.
- `Drivers/sensor/I2C_sensor.c` — the read branch in `I2C_read_data()`.
- `Projects/Applications/DRAGINO-LRWAN-AT/project.uvprojx` — registers `sht40.c`
  in the Keil build.
- `utils/SHT40_test/SHT40_test.ino` — a standalone Arduino/ESP32 sketch (uses
  `Wire`) that talks to an SHT-40 with the same protocol, to bench-test the sensor.

---

# Building from the command line (GCC + make)

Besides Keil MDK (`project.uvprojx`), the firmware can be built with the
GNU Arm Embedded toolchain (`arm-none-eabi-gcc`) and GNU `make`:

```sh
cd Projects/Applications/DRAGINO-LRWAN-AT
make TREMO_SDK_PATH=<repo root> TOOLCHAIN_PATH=<path to arm-none-eabi/bin/>
```

Output: `Make_out/DRAGINO-LRWAN-AT.bin` (and `.hex`). The LoRaWAN region is set in
the Makefile via `-DREGION_AU915` (change it to your region, e.g. `REGION_EU868`,
`REGION_US915`).

# Flashing over UART

Put the board into download mode (**hold BOOT/PRG, press RESET, release BOOT**),
then flash the application:

```sh
python build/scripts/tremo_loader.py -p <COM port> -b 921600 flash 0x0800D000 Make_out/DRAGINO-LRWAN-AT.bin
```

The application is flashed at `0x0800D000` (above the bootloader). Do **not**
overwrite `0x08000000`, which holds the Dragino bootloader.
