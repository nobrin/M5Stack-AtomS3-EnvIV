// 画面表示
#include <M5GFX.h>
#include "UnitEnvIV.h"

#define IMG_WIFI_OK   "/img/wifi_ok_sm.png"           // WiFi接続中
#define IMG_WIFI_NO   "/img/wifi_no_sm.png"           // WiFi未接続

#define IMG_BT_BLUE   "/img/bluetooth_blue_sm.png"    // GATT接続中
#define IMG_BT_GRAY   "/img/bluetooth_gray_sm.png"    // BTなし
#define IMG_BT_WHITE  "/img/bluetooth_white_sm.png"   // BT広告中

constexpr int DRAW_OFFSET_Y = 0;    // 下にする場合は46
constexpr int PLOT_OFFSET_Y = 87;   // プロットエリアのオフセット

class AtomS3Display {
  private:
    M5Canvas *pCanvas;

  public:
    void begin();
    void update();
    void drawFrame(int offsetY=DRAW_OFFSET_Y);
    void drawMeasurement(Measurement *m, int offsetY=DRAW_OFFSET_Y);
    void drawStatus(bool useBle, bool isBleConn, bool isWifi, bool isNtp, bool isMqtt, int offsetY=PLOT_OFFSET_Y);
    void drawDot(int offsetY=PLOT_OFFSET_Y);
    void drawPublished(int offsetY=PLOT_OFFSET_Y);
    void drawClock(int offsetY);
    void plotGraph(Measurement history[], unsigned short historyLength, int offsetY=PLOT_OFFSET_Y);
};

#pragma once
