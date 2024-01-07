// 画面表示
// オフセットはdisplay.hでディフォルト値を設定
#include <SPIFFS.h>
#include <M5Unified.h>
#include "display.h"

void AtomS3Display::begin() {
  // 初期化する
  this->pCanvas = new M5Canvas(&M5.Display);
  this->pCanvas->createSprite(128, 128);
  this->pCanvas->clear(TFT_BLACK);
}

void AtomS3Display::update() {
  // canvasを画面に反映させる
  this->pCanvas->pushSprite(0, 0);
}

void AtomS3Display::drawFrame(int offsetY) {
  // 変わらない部分を描画
  this->pCanvas->setTextColor(TFT_DARKGRAY);
  this->pCanvas->setFont(&lgfxJapanGothic_16);
  this->pCanvas->setTextSize(1);
  this->pCanvas->drawRightString("℃", 63, 12 + offsetY);
  this->pCanvas->drawRightString("％", 127, 12 + offsetY);
  this->pCanvas->drawRightString("hPa", 127, 66 + offsetY);

  this->pCanvas->setFont(&Font0);
  this->pCanvas->setCursor(0, 0 + offsetY);
  this->pCanvas->print("Temp.");
  this->pCanvas->setCursor(67, 0 + offsetY);
  this->pCanvas->print("Humidity");
  this->pCanvas->setCursor(104, 54 + offsetY);
  this->pCanvas->print("Atm.");

  this->pCanvas->drawFastVLine(64, 0 + offsetY, 50, TFT_DARKGRAY);
  this->pCanvas->drawFastHLine(0, 49 + offsetY, 128, TFT_DARKGRAY);

  if(offsetY >= 2){ this->pCanvas->drawFastHLine(0, offsetY - 2, 128, TFT_DARKGRAY); }
  this->update();
}

void AtomS3Display::drawMeasurement(Measurement *m, int offsetY) {
  // 測定データの描画
  char buf[10];
  this->pCanvas->setFont(&Font7);

  // 温度表示
  this->pCanvas->setTextSize(0.7);
  this->pCanvas->setTextColor(TFT_ORANGE, TFT_BLACK);
  sprintf(buf, "%2d", (int)m->temperature);
  this->pCanvas->drawRightString(buf, 45, 12 + offsetY);

  this->pCanvas->setTextSize(0.3);
  sprintf(buf, ".%d", (int)((m->temperature - (int)m->temperature) * 10));
  this->pCanvas->drawRightString(buf, 61, 31 + offsetY);

  // 湿度表示
  this->pCanvas->setTextSize(0.7);
  this->pCanvas->setTextColor(TFT_SKYBLUE, TFT_BLACK);
  sprintf(buf, "%2d", (int)m->humidity);
  this->pCanvas->drawRightString(buf, 111, 12 + offsetY);

  this->pCanvas->setTextSize(0.3);
  sprintf(buf, ".%d", (int)((m->humidity - (int)m->humidity) * 10));
  this->pCanvas->drawRightString(buf, 127, 31 + offsetY);

  // 気圧表示
  this->pCanvas->setTextSize(0.6);
  this->pCanvas->setTextColor(TFT_DARKGREEN, TFT_BLACK);
  sprintf(buf, "%.1f", m->pressure);
  this->pCanvas->drawRightString(buf, 100, 52 + offsetY);

  this->update();
}

void AtomS3Display::drawStatus(bool useBle, bool isBleConn, bool isWifi, bool isNtp, bool isMqtt, int offsetY) {
  // 透過部が重複されていくので画像消去
  this->pCanvas->fillRect(102, offsetY, 26, 41, TFT_BLACK);

  // Bluetoothアイコンの描画
  if(useBle){
    if(isBleConn){ this->pCanvas->drawPngFile(SPIFFS, IMG_BT_BLUE, 102, offsetY); }
    else{ this->pCanvas->drawPngFile(SPIFFS, IMG_BT_WHITE, 102, offsetY); }
  }else{
    this->pCanvas->drawPngFile(SPIFFS, IMG_BT_GRAY, 102, offsetY);
  }

  // WiFiアイコンの描画
  if(isWifi){ this->pCanvas->drawPngFile(SPIFFS, IMG_WIFI_OK, 113, offsetY); }
  else{ this->pCanvas->drawPngFile(SPIFFS, IMG_WIFI_NO, 113, offsetY); }

  this->pCanvas->setFont(&Font0);
  this->pCanvas->setTextSize(1);

  this->pCanvas->setTextColor(isNtp ? TFT_LIGHTGRAY : TFT_RED, TFT_BLACK);
  this->pCanvas->drawCenterString("NTP", 115, 22 + offsetY);

  this->pCanvas->setTextColor(isMqtt ? TFT_LIGHTGRAY : TFT_RED, TFT_BLACK);
  this->pCanvas->drawCenterString("MQTT", 115, 33 + offsetY);

  this->update();
}

void AtomS3Display::drawDot(int offsetY) {
  // 動作インジケータ
  this->pCanvas->fillCircle(126, 16 + offsetY, 1, TFT_DARKGRAY);
  this->update();
}

void AtomS3Display::drawPublished(int offsetY) {
  // MQTTをパブリッシュしたときに色を変える
  this->pCanvas->setFont(&Font0);
  this->pCanvas->setTextSize(1);
  this->pCanvas->setTextColor(TFT_GREEN, TFT_BLACK);
  this->pCanvas->drawCenterString("MQTT", 115, 33 + offsetY);
  this->update();
}

void AtomS3Display::drawClock(int offsetY) {
  // 時計部の描画
  this->pCanvas->setFont(&Font7);
  this->pCanvas->setTextColor(TFT_WHITE, TFT_BLACK);
  this->pCanvas->setTextSize(0.7);
  this->pCanvas->setCursor(4, 4);

  struct tm dt;
  getLocalTime(&dt, 10);  // Arg2はシンクロ用？(時刻を合わせてたらすぐだが、過去のままだと時間が掛かる)
  this->pCanvas->printf("%02d:%02d", dt.tm_hour, dt.tm_min);

  this->update();
}

void AtomS3Display::plotGraph(Measurement history[], unsigned short historyLength, int offsetY) {
  // トレンドグラフを描く
  M5Canvas frame(this->pCanvas);  // フレーム部分のCanvas
  frame.createSprite(96, 45);
  frame.fillSprite(TFT_BLACK);
  frame.setFont(&Font0);
  frame.setTextSize(1);
  frame.setTextColor(TFT_DARKGRAY);
  frame.drawString("+5", 0, 0);
  frame.drawString("-5", 0, 25);
  frame.drawRect(14, 0, 82, 33, TFT_BROWN);

  M5Canvas plot(&frame);        // プロット部分のCanvas
  plot.createSprite(80, 31);
  plot.fillSprite(TFT_BLACK);
  plot.drawFastHLine(0, 15, 80, plot.color888(16, 16, 16));

  unsigned short sz = historyLength;
  Measurement *cur = &history[sz - 1];    // 最新の測定結果
  Measurement *m1, *m2;

  if(cur->measuredAt.tm_year == 0){ return; }   // 測定されていない
  for(int i = sz - 1; i >= 1; i--){
    // プロットする
    int y1, y2;
    m1 = &history[i];
    m2 = &history[i - 1];
    if(m2->measuredAt.tm_year == 0){ m2 = m1; break; }  // データが入っていない

    // 温度プロット
    y1 = 14 - (int)((m1->temperature - cur->temperature) * 3);
    y2 = 14 - (int)((m2->temperature - cur->temperature) * 3);
    plot.drawLine(i, y1, i - 1, y2, TFT_ORANGE);

    // 湿度プロット
    y1 = 15 - (int)((m1->humidity - cur->humidity) * 3);
    y2 = 15 - (int)((m2->humidity - cur->humidity) * 3);
    plot.drawLine(i, y1, i - 1, y2, TFT_SKYBLUE);

    // 気圧プロット
    y1 = 16 - (int)((m1->pressure - cur->pressure) * 3);
    y2 = 16 - (int)((m2->pressure - cur->pressure) * 3);
    plot.drawLine(i, y1, i - 1, y2, TFT_DARKGREEN);
  }
  plot.pushSprite(15, 1); // フレーム側に反映

  // 測定データの時刻をフレームに出力し画面に反映させる
  char buf[6];
  sprintf(buf, "%02d:%02d", m2->measuredAt.tm_hour, m2->measuredAt.tm_min);
  frame.drawString(buf, 15, 34);
  sprintf(buf, "%02d:%02d", cur->measuredAt.tm_hour, cur->measuredAt.tm_min);
  frame.drawRightString(buf, 96, 34);
  frame.pushSprite(0, offsetY);

  if(offsetY >= 84){ this->pCanvas->drawFastHLine(0, offsetY - 4, 128, TFT_DARKGRAY); }
  this->update();
}
