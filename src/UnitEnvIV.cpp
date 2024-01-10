/*
 * Unit EnvIV用クラス
 */
#include "UnitEnvIV.h"

void Measurement::appendInt(std::string *data, uint16_t value) {
  // 送信データに2バイトIntを追加する
  *data += (uint8_t)(value & 0xff);         // 下位バイト
  *data += (uint8_t)((value >> 8) & 0xff);  // 上位バイト
}

std::string Measurement::getBLEData(uint8_t seq) {
  // BLEで送信するためのデータフレームを作成する
  std::string s = "";
//  s += (char)0xff;   // AD type 0xFF: Manufacturer specific data (NimBLEでは不要)
  s += (char)0xff;   // Test manufacturer ID low byte
  s += (char)0xff;   // Test manufacturer ID high byte
  s += (char)seq;  // シーケンス番号
  this->appendInt(&s, (uint16_t)(this->temperature * 100));  // 温度
  this->appendInt(&s, (uint16_t)(this->humidity * 100));     // 湿度
  this->appendInt(&s, (uint16_t)(this->pressure * 10));      // 気圧
  return s;
}

void UnitEnvIV::begin() {
  // センサーの初期化
  this->SHT4X.begin(Wire, 0x44);  // 温湿度センサー
  this->BMP280.begin(0x76);       // 気圧センサー
  this->BMP280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                           Adafruit_BMP280::SAMPLING_X2,
                           Adafruit_BMP280::SAMPLING_X16,
                           Adafruit_BMP280::FILTER_X16,
                           Adafruit_BMP280::STANDBY_MS_500);
}

void UnitEnvIV::update(Measurement *m) {
  // 計測データをアップデートする
  getLocalTime(&m->measuredAt, 10);
  this->SHT4X.measureHighPrecision(m->temperature, m->humidity);
  m->pressure = this->BMP280.readPressure() / 100;
}

void UnitEnvIV::record() {
  // 現在の測定結果を記録する
  memcpy(&this->history[0], &this->history[1], sizeof(Measurement) * 79);
  Measurement meas;
  this->update(&meas);
  this->history[79] = meas;
}
