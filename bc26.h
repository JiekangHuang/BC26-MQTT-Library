#ifndef BC26_H
#define BC26_H
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>

#define DEBUG_MODE true
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

bool BC26Init(long baudrate, String apn, int band);
bool BC26ConnectMQTTServer(String host, String user, String key, int port);
bool BC26MQTTPublish(String topic, String msg, int qos);
bool BC26MQTTSubscribe(String topic, int qos);
int  getBC26CSQ(void);
bool readBC26MQTTMsg(String topic, String &msg);

#endif /* BC26_H */