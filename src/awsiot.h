// MQTTへの接続
#include <WString.h>
#include <MQTTClient.h>

bool loadFile(String *str, const char *fullpath);
bool connectAWSIoT(MQTTClient *mqtt, const char *topic, const char *endpoint, int port,
                   const char *devName, const char *rootCa, const char *devCert, const char *devKey,
                   MQTTClientCallbackSimple cb);

#pragma once
