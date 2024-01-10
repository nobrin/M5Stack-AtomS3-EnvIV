// BLE初期化
#include <NimBLEDevice.h>
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
                                                  /* AruduinoBLE用
                                                   BLECharacteristic::PROPERTY_READ |
                                                   BLECharacteristic::PROPERTY_WRITE |
                                                   BLECharacteristic::PROPERTY_NOTIFY |
                                                   BLECharacteristic::PROPERTY_INDICATE);
                                                   */
                                                   NIMBLE_PROPERTY::READ |
                                                   NIMBLE_PROPERTY::WRITE |
                                                   NIMBLE_PROPERTY::NOTIFY |
                                                   NIMBLE_PROPERTY::INDICATE);

  pCharacteristic->setCallbacks(chrCallbacks);
//  pCharacteristic->addDescriptor(new BLE2902());  // NimBLEでは不要
  pService->start();
}

void updateAdvertisementData(BLEServer *pServer, std::string *data) {
  // 環境情報をセットする(NimBLE用)
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setManufacturerData(*data);
  pAdvertising->stop();
  pAdvertising->start();
}
