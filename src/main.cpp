/*
  M5Stack AtomS3 + UNIT ENV.IVでBLE/MQTTの温湿度、気圧計
 */

#include <SPIFFS.h>     // M5Unified.hより前に配置する
#include <WiFi.h>
#include <Wire.h>
#include <M5Unified.h>

#include <NimBLEDevice.h>
#include <MQTT.h>
#include <Ticker.h>
#include <base64.hpp>

#include "UnitEnvIV.h"
#include "display.h"
#include "ble.h"
#include "awsiot.h"
#include "util.h"

#define CONFIG_FILE "/config.txt"     // MyConfigに読み込むファイル
#define NTP_SERVER1 "0.pool.ntp.org"
#define NTP_SERVER2 "1.pool.ntp.org"
#define NTP_SERVER3 "2.pool.ntp.org"
const bool useNTP = true;   // 開発時はfalseにする(NTPに余計な負荷をかけないため)

const BLEUUID SERVICE_UUID("21c134bf-dd8b-44f5-9966-8829f720a556");
const BLEUUID CHARACTERISTIC_UUID("95e1977b-fd27-4587-9d5e-f2a56352a177");

constexpr uint8_t TIMES_FOR_UPDATE_DISPLAY = 30;  // 測定結果の更新間隔(秒)
constexpr uint8_t TIMES_FOR_PUBLISH = 60;         // MQTT発行間隔(秒)

RTC_DATA_ATTR static uint8_t g_seq; // remember number of boots in RTC Memory

struct {  // タイマーと共有するための状態
  bool ntp = false;
  bool mqtt = false;
} Status;

AtomS3Display FB;           // 画面更新用フレームバッファ操作オブジェクト
BLEServer *pServer = NULL;  // BLEサーバー
UnitEnvIV EnvIV;            // Unit EnvIV操作オブジェクト
Ticker ticker;              // タイマー
MQTTClient MQTT;            // MQTTクライアント
MyConfig Cfg;               // 設定の構造体

void onMessage(String &topic, String &payload) {
  // MQTTメッセージ受信時のハンドラ
  Serial.printf("Message: %s\n", payload.c_str());
}

class MyServerCallbacks: public BLEServerCallbacks {
  // BLE GATT: サーバー側コールバック
  void onConnect(BLEServer *pServer) {
    // GATT接続された
    Serial.println("Connected with GATT.");
  }

  void onDisconnect(BLEServer *pServer) {
    // GATT接続が閉じられた
    Serial.println("Disconnected with GATT.");
    pServer->getAdvertising()->start(); // 広告再開(onConnect時に広告が停止されている)
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  // BLE GATT: キャラクタリスティックコールバック
  void onRead(BLECharacteristic *pCharacteristic) {
    // 読込要求に対してデータを構成して送信
    // GATT接続時、Notifyで情報送信するため読むのは最初の一回だけ
    Measurement m;
    EnvIV.update(&m);
    std::string data = m.getBLEData(g_seq);
    pCharacteristic->setValue(data);
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    // 書込要求があった(ここでは書き込まれたデータをそのまま返す)
    std::string value = pCharacteristic->getValue();
  }
};

void detectRotation(Measurement *meas) {
  // 本体の向きに合わせて画面を回転させて再描画
  uint8_t rot = M5.Display.getRotation();
  float ax, ay, az;
  M5.Imu.getAccel(&ax, &ay, &az);
  if(ax >  0.6){ rot = 1; }
  else if(ay < -0.6){ rot = 2; }
  else if(ax < -0.6){ rot = 3; }
  else if(ay >  0.6){ rot = 0; }

  if(rot != M5.Display.getRotation()){
    // 変化があれば再描画
    M5.Display.setRotation(rot);
    FB.update();
  }
}

void onTimer() {
  // Tickerによる実行
  Measurement meas;
  static unsigned short loopCount = 0;

  M5.update();
  EnvIV.update(&meas);  // センサーの値を読み取る
  MQTT.loop();

  // WiFi, BT, NTP, MQTTのステータスを描く
  FB.drawStatus(Cfg.useBle, pServer->getConnectedCount(), WiFi.isConnected(), Status.ntp, Status.mqtt);
  if(loopCount % 2 == 0){ FB.drawDot(); }             // 1秒ごとにドットを点滅(死活確認用)

  if(loopCount % TIMES_FOR_UPDATE_DISPLAY == 0){      // 30秒に一度更新する
    FB.drawMeasurement(&meas);                        // 測定結果の更新
    EnvIV.record();                                   // 履歴を追加
    FB.plotGraph(EnvIV.history, EnvIV.historyLength); // トレンドグラフを描く
  }

  if(loopCount % TIMES_FOR_PUBLISH == 0){ // 60秒に一度パブリッシュする
    // MQTTにパブリッシュする
    if(Status.mqtt){
      Serial.print("MQTT Publishing...");
      char msg[255];
      sprintf(msg,
              "{\"device_id\":\"%s\", \"temperature\":%.1f,\"humidity\":%.1f, \"pressure\":%.1f}",
              Cfg.mqttDeviceName, meas.temperature, meas.humidity, meas.pressure);
      MQTT.publish(Cfg.mqttTopic, msg);
      FB.drawPublished();   // MQTTパブリッシュ時に"MQTT"文字の色を変える
      Serial.println("DONE!!");
    }
  }

  if(pServer->getConnectedCount()){
    // GATT接続がある場合はNotifyする
    std::string data = meas.getBLEData(g_seq);  // データを構成する
    BLEService *pService = pServer->getServiceByUUID(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);
    pCharacteristic->setValue(data);
    pCharacteristic->notify();
  }

  // BLE広告データを更新する
  std::string data = meas.getBLEData(g_seq);
  updateAdvertisementData(pServer, &data);

  // 本体の向きに合わせて画面を回転させる
  detectRotation(&meas);

  Serial.printf("Loop: %d\n", loopCount);

  g_seq++;      // シーケンス番号をインクリメント
  loopCount++;  // ループカウントを更新
  if(loopCount >= 3600){ loopCount = 0; }
}

void setup() {
  // 初期化
  M5.begin();
  Wire.begin();
  SPIFFS.begin();
  EnvIV.begin();  // Unit EnvIVを使う
  FB.begin();

  M5.Display.setBrightness(60);
  FB.drawFrame();

  // 設定を読み込む
  loadConfig(CONFIG_FILE, &Cfg);

  // BLEの初期化
  BLEDevice::init("m5stack");
  pServer = BLEDevice::createServer();
  initAdvertising(pServer);
  startService(pServer,
               SERVICE_UUID, CHARACTERISTIC_UUID,
               new MyServerCallbacks(), new MyCallbacks());
  if(Cfg.useBle){
    pServer->getAdvertising()->start();   // 広告開始
  }

  // WiFiの初期化
  if(Cfg.wifissid != ""){
    // WiFiへの接続とNTPによる時刻合わせを開始
    WiFi.begin(Cfg.wifissid, Cfg.wifipass);
    setenv("TZ", Cfg.timezone.c_str(), 1);
    tzset();
    if(useNTP) {
      configTzTime(Cfg.timezone.c_str(), NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    }
  }

  // 1秒ごとにタイマーを実行する
  ticker.attach(1, onTimer);
}

void loop() {
  // ループではWiFiなどのチェックを行う
  if(WiFi.isConnected()){
    if(!Status.ntp){ Status.ntp = useNTP; }
    if(!Status.mqtt && Cfg.mqttEndpoint != ""){
      Status.mqtt = connectAWSIoT(&MQTT,
                                  Cfg.mqttTopic.c_str(), Cfg.mqttEndpoint.c_str(), Cfg.mqttPort,
                                  Cfg.mqttDeviceName.c_str(),
                                  Cfg.mqttRootCA.c_str(), Cfg.mqttDeviceCert.c_str(),
                                  Cfg.mqttDeviceKey.c_str(),
                                  onMessage);
    }
  }
  delay(1000);
}
