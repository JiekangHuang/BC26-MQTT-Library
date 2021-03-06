#include "bc26.h"

// #define apn "<YOUR APN>"
// #define host "io.adafruit.com"
// #define user "<YOUR USERNAME>"
// #define key "<YOUR ACTIVE KEY>"
// const char *topic = "<YUOR TOPIC>";

const char *apn   = "<YOUR APN>";
const char *host  = "io.adafruit.com";
const char *user  = "<YOUR USERNAME>";
const char *key   = "<YOUR ACTIVE KEY>";
const char *topic = "<YOUT TOPIC>";

void setup()
{
    Serial.begin(BAUDRATE_9600);

    // bc26 init
    BC26Init(BAUDRATE_9600, apn, BAND_8);
    // connect to server
    BC26ConnectMQTTServer(host, user, key, MQTT_PORT_1883);
}

void loop()
{
    char        buff[100];
    static long timer = millis();
    if (millis() - timer >= 10000) {
        timer = millis();
        sprintf(buff, "%ld", timer / 1000);
        BC26MQTTPublish(topic, buff, MQTT_QOS0);
    }
}
