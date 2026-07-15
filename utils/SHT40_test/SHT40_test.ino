/*
 * SHT40_test.ino  -  Teste do sensor Sensirion SHT-40 na Arduino IDE
 *
 * Le temperatura e umidade do SHT-40 (I2C 0x44) usando a biblioteca Wire.
 * Usa o MESMO protocolo do driver do firmware SN50v3 (Drivers/sensor/sht40.c):
 *   - comando de medida de 1 byte (alta/media/baixa precisao)
 *   - leitura de 6 bytes: [T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC]
 *   - CRC-8 Sensirion (polinomio 0x31, init 0xFF)
 *   - T[C]  = -45 + 175 * raw_t  / 65535
 *   - RH[%] =  -6 + 125 * raw_rh / 65535   (limitado a 0..100)
 *
 * Ligacao (padrao Wire):
 *   - UNO/Nano:  SDA=A4, SCL=A5
 *   - ESP32:     SDA=21, SCL=22 (ajuste em Wire.begin() se necessario)
 *   - Alimente o SHT-40 com 3.3V e use pull-ups de ~4.7k em SDA/SCL.
 */

#include <Wire.h>

#define SHT40_ADDR          0x44

// Comandos de medida (escolha a precisao)
#define SHT40_MEAS_HIGH     0xFD   // alta   (~8.2 ms)
#define SHT40_MEAS_MEDIUM   0xF6   // media  (~4.5 ms)
#define SHT40_MEAS_LOW      0xE0   // baixa  (~1.6 ms)

#define SHT40_READ_SERIAL   0x89
#define SHT40_SOFT_RESET    0x94

// >>> Troque aqui a precisao usada no teste <<<
uint8_t  measCmd   = SHT40_MEAS_HIGH;
uint16_t measDelay = 10;           // ms de espera apos o comando de medida

// CRC-8 Sensirion (poly 0x31, init 0xFF) sobre 2 bytes
uint8_t sht40_crc(uint8_t b0, uint8_t b1) {
  uint8_t crc = 0xFF;
  uint8_t data[2] = { b0, b1 };
  for (uint8_t i = 0; i < 2; i++) {
    crc ^= data[i];
    for (uint8_t bit = 8; bit > 0; bit--) {
      if (crc & 0x80) crc = (crc << 1) ^ 0x31;
      else            crc = (crc << 1);
    }
  }
  return crc;
}

// Envia 1 comando de 1 byte
bool sht40_cmd(uint8_t cmd) {
  Wire.beginTransmission(SHT40_ADDR);
  Wire.write(cmd);
  return (Wire.endTransmission() == 0);
}

// Le a temperatura e a umidade. Retorna true em sucesso.
bool sht40_read(float *temperature, float *humidity) {
  if (!sht40_cmd(measCmd)) return false;   // dispara a medida
  delay(measDelay);

  if (Wire.requestFrom(SHT40_ADDR, 6) != 6) return false;
  uint8_t d[6];
  for (uint8_t i = 0; i < 6; i++) d[i] = Wire.read();

  // valida CRC das duas palavras
  if (sht40_crc(d[0], d[1]) != d[2]) return false;
  if (sht40_crc(d[3], d[4]) != d[5]) return false;

  uint16_t raw_t  = ((uint16_t)d[0] << 8) | d[1];
  uint16_t raw_rh = ((uint16_t)d[3] << 8) | d[4];

  float t  = -45.0f + 175.0f * (float)raw_t  / 65535.0f;
  float rh =  -6.0f + 125.0f * (float)raw_rh / 65535.0f;
  if (rh > 100.0f) rh = 100.0f;
  if (rh <   0.0f) rh =   0.0f;

  *temperature = t;
  *humidity    = rh;
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}
  Wire.begin();                 // ESP32: Wire.begin(21, 22);
  Wire.setClock(100000);

  Serial.println();
  Serial.println(F("=== Teste SHT-40 (I2C 0x44) ==="));

  // Le o numero de serie para confirmar a comunicacao
  if (sht40_cmd(SHT40_READ_SERIAL)) {
    delay(10);
    if (Wire.requestFrom(SHT40_ADDR, 6) == 6) {
      uint8_t s[6];
      for (uint8_t i = 0; i < 6; i++) s[i] = Wire.read();
      if (sht40_crc(s[0], s[1]) == s[2] && sht40_crc(s[3], s[4]) == s[5]) {
        uint32_t serial = ((uint32_t)s[0] << 24) | ((uint32_t)s[1] << 16) |
                          ((uint32_t)s[3] << 8)  |  (uint32_t)s[4];
        Serial.print(F("SHT-40 detectado. Serial: 0x"));
        Serial.println(serial, HEX);
      } else {
        Serial.println(F("CRC do serial invalido (verifique fiacao/pull-ups)."));
      }
    } else {
      Serial.println(F("Sem resposta ao ler o serial."));
    }
  } else {
    Serial.println(F("Nenhum ACK em 0x44. Verifique fiacao, 3.3V e pull-ups."));
  }
}

void loop() {
  float t, rh;
  if (sht40_read(&t, &rh)) {
    Serial.print(F("SHT4x_temp: "));
    Serial.print(t, 1);
    Serial.print(F(" C, SHT4x_hum: "));
    Serial.print(rh, 1);
    Serial.println(F(" %"));
  } else {
    Serial.println(F("Falha na leitura (sem ACK / CRC / fiacao)."));
  }
  delay(2000);
}
