/*
 * Unit EnvIV用クラス
 */
#define ENV_IV_HISTORY_SIZE 80;

#include <string>
#include <SensirionI2CSht4x.h>
#include <Adafruit_BMP280.h>

class Measurement {
  private:
    void appendInt(std::string *data, uint16_t value);

  public:
    struct tm measuredAt;     // 測定時刻
    float temperature;        // 温度(℃)
    float humidity;           // 湿度(%)
    float pressure;           // 気圧(hPa)
    std::string getBLEData(uint8_t seq);  // BLE用データフレームの生成
};

class UnitEnvIV {
  private:
    SensirionI2CSht4x SHT4X;  // 温湿度 SHT40
    Adafruit_BMP280 BMP280;   // 気圧 BMP280

  public:
    const static uint8_t historyLength = ENV_IV_HISTORY_SIZE;
    Measurement history[historyLength];  // 測定履歴を保存する

    void begin();
    void update(Measurement *m);
    void record();
};

#pragma once
