#ifndef BC26_H
#define BC26_H
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>

#define DEBUG_MODE false
#define debugSerial Serial
#if DEBUG_MODE
#define DEBUG_PRINT(x) debugSerial.println(x)
#else
#define DEBUG_PRINT(x)
#endif
#define ERROR(x) debugSerial.println(x)

#define BAUDRATE_9600 9600
#define BAUDRATE_19200 19200
#define BAUDRATE_38400 38400

#define MQTT_PORT_1883 1883

typedef enum
{
    MQTT_QOS0,
    MQTT_QOS1,
} MQTT_QOS_E;

typedef enum
{
    BAND_1  = 1,
    BAND_3  = 3,
    BAND_8  = 8,
    BAND_28 = 28,
} BC26_BAND;

bool BC26Init(long baudrate, const char *apn, int band);
bool BC26ConnectMQTTServer(const char *host, const char *user, const char *key, int port);
bool BC26MQTTPublish(const char *topic, char *msg, int qos);
bool BC26MQTTSubscribe(const char *topic, int qos);
int  getBC26CSQ(void);
bool readBC26MQTTMsg(const char *topic, char *msg);

#endif /* BC26_H */