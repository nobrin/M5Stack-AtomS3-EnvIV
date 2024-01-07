// BLE初期化
#include <BLEServer.h>

void initAdvertising(BLEServer *pServer);
void startService(BLEServer *pServer,
                  BLEUUID serviceUUID, BLEUUID chrUUID,
                  BLEServerCallbacks *svrCallbacks, BLECharacteristicCallbacks *chrCallbacks);
void updateAdvertisementData(BLEServer *pServer, std::string *data);

#pragma once
