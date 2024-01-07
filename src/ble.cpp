// BLE初期化
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include <BLE2902.h>
#include "ble.h"

void initAdvertising(BLEServer *pServer) {
  // アドバタイズを初期化する
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setMinInterval(256);  // default 0x20
  pAdvertising->setMaxInterval(512);  // default 0x40
}

void startService(BLEServer *pServer, BLEUUID serviceUUID, BLEUUID chrUUID,
                  BLEServerCallbacks *svrCallbacks, BLECharacteristicCallbacks * chrCallbacks) {
  // サービスを初期化し、開始する(GATT接続可能になる)
  pServer->setCallbacks(svrCallbacks);

  // サービスおよびキャラクタリスティックの設定
  BLEService *pService = pServer->createService(serviceUUID);
  BLECharacteristic *pCharacteristic;
  pCharacteristic = pService->createCharacteristic(chrUUID,
                                                   BLECharacteristic::PROPERTY_READ |
                                                   BLECharacteristic::PROPERTY_WRITE |
                                                   BLECharacteristic::PROPERTY_NOTIFY |
                                                   BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic->setCallbacks(chrCallbacks);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
}

void updateAdvertisementData(BLEServer *pServer, std::string *data) {
  // 環境情報をセットする
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x06);
  *data = (char)data->length() + *data; // 先頭に長さを追加
  oAdvertisementData.addData(*data);
  pServer->getAdvertising()->setAdvertisementData(oAdvertisementData);
}
