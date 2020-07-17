#include "bc26.h"

const char *apn  = "<YOUR APN>";
const char *host = "io.adafruit.com";
const char *user = "<YOUR USERNAME>";
const char *key  = "<YOUR ACTIVE KEY>";

const char *topic_1 = "<YUOR TOPIC 1>";
const char *topic_2 = "<YUOR TOPIC 2>";
const char *topic_3 = "<YUOR TOPIC 3>";

void callback1(char *msg);
void callback2(char *msg);
void callback3(char *msg);

void setup()
{
    Serial.begin(BAUDRATE_9600);
    // bc26 init
    BC26Init(BAUDRATE_9600, apn, BAND_8);
    // connect to server
    BC26ConnectMQTTServer(host, user, key, MQTT_PORT_1883);
    // Subscribe topic
    BC26MQTTSubscribe(topic_1, MQTT_QOS0, callback1);
    BC26MQTTSubscribe(topic_2, MQTT_QOS0, callback2);
    BC26MQTTSubscribe(topic_3, MQTT_QOS0, callback3);
}

void loop()
{
    ProcSubs();
}

void callback1(char *msg)
{
    Serial.print(F("topic_1: "));
    Serial.println(msg);
}

void callback2(char *msg)
{
    Serial.print(F("topic_2: "));
    Serial.println(msg);
}

void callback3(char *msg)
{
    Serial.print(F("topic_3: "));
    Serial.println(msg);
}
