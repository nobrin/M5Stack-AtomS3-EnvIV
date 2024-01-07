# M5Stack-AtomS3-EnvIV

AtomS3と[EnvIVユニット](https://docs.m5stack.com/ja/unit/ENV%E2%85%A3%20Unit)を使った温湿度・気圧計です。

使い方を学ぶためにいろいろ実装したので、BLEによるデータのブロードキャストおよび接続モード、MQTTへの通知を行います。

各種設定はSPIFFS上の `/config.txt` で行います。


## 対応機器

M5Stack AtomS3 + M5Stack用温湿度気圧センサユニットVer.4(ENV IV)で動作確認を行っています。M5Unifiedを使用しているので、他のM5Stackでも動作可能だと思います。画面出力はAtomS3に合わせて128x128です。


## 使い方

AtomS3のPortAにEnvIVを接続し、ビルドしたファームと必要なファイルと書き込めば動作します。SPIFFSに書き込むファイルは `data` ディレクトリ内のファイルに以下のファイルを用意します。

- /config.txt -- 設定ファイル(config.txt.sampleを参照)
- (MQTT使用時)AWS IoT Coreのデバイス証明書およびデバイス秘密鍵


### 設定ファイルの書き方

設定ファイルには無線LAN設定、BLEの使用およびMQTTの設定を書き込みます。以下のような内容です。ディフォルトではAmazon Root CA 1のファイルが設定してあるため、Root CAの設定は不要です。また、ディフォルトではBLEは無効になっていますので、BLEを使用したい場合は `1` にして下さい。

    ssid=WIFISSID
    pass=WIFIPASSPHRASE
    ble=0
    endpoint=xxxxx-ats.iot.REGION.amazonaws.com
    port=8883
    devname=M5Stack
    devcert=/M5Stack.crt
    devkey=/M5Stack.key
    topic=M5Stack/EnvIV

AWS IoT Coreで"モノ"の設定を行い、接続に必要なデバイス証明書とデバイス秘密鍵も `data` に入れておきます。
MQTTを使わない場合は、NTPによる時刻設定を行うために、 `ssid` および `pass` の行のみ指定します。

ファイルが準備できたら、PlatformIOの `Upload Filesystem Image` などでSPIFFSに書き込みます。


### 起動後

起動後は自動でWiFiに接続し、NTPによる時刻調整を行います。MQTTの設定を行っている場合は接続します。また、BLEを有効にした場合、BLEによるアドバタイズを開始します。

BLEアドバタイズではデバイス名 `m5stack` でブロードキャストされます。データには温度、湿度、気圧が含まれるため、アドバタイズを受信し、読むだけで現在の測定データを知ることができます。受信サンプルは `client/client.py` を参照して下さい。BLE用外部モジュールとして `Bleak` を使用しています。

GATT通信を行うことができます。接続した場合、1秒に1回程度の割合で現在の測定データが通知されます。

HTMLによるクライアントサンプルは https://nobrin.github.io/M5Stack-AtomS3-EnvIV/ble.html です。BLEデバイス名 `m5stack` に接続できます。ChromeまたはEdgeでお試しください。


## 制限事項

M5Unifiedによるコーディングを学ぶために書いた程度ですので、例外処理は想定していません。ご自分の責任でご使用下さい。

BLEアドバタイズで電波が届けば誰でも温度、湿度、気圧を受信できてしまうのでお気をつけ下さい。