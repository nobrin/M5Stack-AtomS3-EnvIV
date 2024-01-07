// MQTTへの接続(AWS IoT Core)
#include <SPIFFS.h>
#include <MQTTClient.h>
#include <WiFiClientSecure.h>
#include "awsiot.h"

bool loadFile(String *str, const char *fullpath) {
  // SPIFFSからファイル内容を読み込みStringに格納する
  if(!SPIFFS.exists(fullpath)){ return false; }
  File file = SPIFFS.open(fullpath, FILE_READ);
  while(file.available()){ *str = file.readString(); }
  file.close();
  return true;
}

bool connectAWSIoT(MQTTClient *mqtt, const char *topic, const char *endpoint, int port,
                   const char *devName,
                   const char *rootCa, const char *devCert, const char *devKey,
                   MQTTClientCallbackSimple cb
                   ) {
  // AWS IoT Coreに接続する
  static WiFiClientSecure https;    // MQTTがグローバルのため静的領域に確保
  static bool isFailed = false;     // 接続に失敗したら以降は試行しない
  if(isFailed){ return false; }

  // 証明書の読み込み
  String ROOT_CA, MY_CERT, MY_KEY;
  if(!loadFile(&ROOT_CA, rootCa)){ isFailed = true; return false; }   // サーバーCA証明書
  if(!loadFile(&MY_CERT, devCert)){ isFailed = true; return false; }  // デバイス証明書
  if(!loadFile(&MY_KEY,  devKey)){ isFailed = true; return false; }   // デバイス秘密鍵

  // 証明書などをセット
  https.setCACert(ROOT_CA.c_str());
  https.setCertificate(MY_CERT.c_str());
  https.setPrivateKey(MY_KEY.c_str());

  // MQTTクライアントを設定し、トピックを購読する
  mqtt->begin(endpoint, port, https);
  if(!mqtt->connect(devName)){ isFailed = true; return false; } // 接続失敗
  mqtt->onMessage(cb);              // メッセージ到達時のイベントハンドラ
  mqtt->subscribe(topic);           // トピックを購読する

  return true;
}
