#include "bc26.h"

const char *apn   = "<YOUR APN>";
const char *host  = "io.adafruit.com";
const char *user  = "<YOUR USERNAME>";
const char *key   = "<YOUR ACTIVE KEY>";
const char *topic = "<YUOR TOPIC>";

void setup()
{
    Serial.begin(BAUDRATE_9600);
    // bc26 init
    BC26Init(BAUDRATE_9600, apn, BAND_8);
    // connect to server
    BC26ConnectMQTTServer(host, user, key, MQTT_PORT_1883);
    // Subscribe topic
    BC26MQTTSubscribe(topic, MQTT_QOS0);
}

void loop()
{
    char message[100];
    if (readBC26MQTTMsg(message)) {
        Serial.println(message);
    }
}
