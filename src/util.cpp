// ユーティリティメソッドなど
#include <WString.h>
#include <SPIFFS.h>
#include "util.h"

unsigned short count(String *s, const char delimiter) {
  // String中のdelimiterの数を数える
  unsigned short cnt = 0;
  for(int i = 0;i < s->length(); i++){
    if(s->charAt(i) == delimiter){ cnt++; }
  }
  return cnt;
}

void split(String *src, const char delimiter, String dst[]) {
  // srcをdelimiterで分割し、dstに入れる
  int idx = 0;
  for(int i = 0;i < src->length(); i++){
    if(src->charAt(i) == delimiter){ idx++; }
    else{ dst[idx] += src->charAt(i); }
  }
}

bool loadConfig(const char *fullpath, MyConfig *cfg) {
  // 設定ファイルをSPIFFSから読み込む
  String buf;
  if(!SPIFFS.exists(fullpath)){ return false; }
  File file = SPIFFS.open(fullpath, FILE_READ);
  if(file.available()){ buf = file.readString(); }
  file.close();
  buf.trim();
  buf.replace("\r\n", "\n");
  buf.replace('\r', '\n');

  unsigned short cnt = count(&buf, '\n') + 1;
  String lines[cnt];
  split(&buf, '\n', lines);

  for(int i = 0; i < cnt; i++){
    if(lines[i].indexOf('=') < 0){ continue; }
    String pair[2];
    split(&lines[i], '=', pair);
    pair[0].trim();
    pair[1].trim();
    if(pair[0].startsWith("#")){ continue; }

    if(pair[0] == "ssid"){          cfg->wifissid = pair[1]; }
    else if(pair[0] == "pass"){     cfg->wifipass = pair[1]; }
    else if(pair[0] == "ble"){      cfg->useBle = (bool)pair[1].toInt(); }
    else if(pair[0] == "endpoint"){ cfg->mqttEndpoint = pair[1]; }
    else if(pair[0] == "port"){     cfg->mqttPort = pair[1].toInt(); }
    else if(pair[0] == "root_ca"){  cfg->mqttRootCA = pair[1]; }
    else if(pair[0] == "devname"){  cfg->mqttDeviceName = pair[1]; }
    else if(pair[0] == "devcert"){  cfg->mqttDeviceCert = pair[1]; }
    else if(pair[0] == "devkey"){   cfg->mqttDeviceKey = pair[1]; }
    else if(pair[0] == "topic"){    cfg->mqttTopic = pair[1]; }
  }

  return true;
}

void readWiFiInfo(String *wifissid, String *wifipass) {
  // SPIFFSからWiFi接続情報を得る
  // 1行目：SSID、2行目：パスフレーズ
  *wifissid = *wifipass = "";
  if(!SPIFFS.exists("/wifiinfo.txt")){ return; }
  File file = SPIFFS.open("/wifiinfo.txt", FILE_READ);
  while(file.available()){
    *wifissid = file.readStringUntil('\n');
    *wifipass = file.readString();
  }
  file.close();
  wifissid->trim();
  wifipass->trim();
}
