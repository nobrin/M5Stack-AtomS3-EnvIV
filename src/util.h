// ユーティリティメソッドなど
#include <WString.h>

struct MyConfig {
  String wifissid = "";                         // WiFi SSID
  String wifipass = "";                         // WiFiパスフレーズ
  String timezone = "JST-9";                    // タイムゾーン
  String mqttDeviceName = "M5Stack";            // MQTTデバイス名
  String mqttEndpoint = "";                     // MQTTエンドポイント
  int mqttPort = 8883;                          // MQTTサービスポート番号
  String mqttRootCA = "/AmazonRootCA1.pem";     // サーバーCA証明書
  String mqttDeviceCert = "";                   // デバイス証明書
  String mqttDeviceKey = "";                    // デバイス秘密鍵
  String mqttTopic = "";                        // MQTTトピック名
  bool useBle = false;                          // BLEを使用するか
};

unsigned short count(String *s, const char delimiter);
void split(String *src, const char delimiter, String dst[]);
bool loadConfig(const char *fullpath, MyConfig *cfg);

#pragma once
