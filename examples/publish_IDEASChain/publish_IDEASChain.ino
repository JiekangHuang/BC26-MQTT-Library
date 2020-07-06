#include "bc26.h"

const char *apn   = "<YOUR APN>";
const char *host  = "iiot.ideaschain.com.tw";
const char *user  = "<YOUR ACCESS TOKEN>";
const char *key   = "";     // empty
const char *topic = "v1/devices/me/telemetry";

void setup()
{
    Serial.begin(BAUDRATE_38400);
    // bc26 init
    BC26Init(BAUDRATE_38400, apn, BAND_8);
    // connect to server
    BC26ConnectMQTTServer(host, user, key, MQTT_PORT_1883);
}

void loop()
{
    static unsigned long timer = millis();
    char                 buff[100];
    if (millis() - timer >= 10000) {
        timer = millis();
        sprintf(buff, "{\"boot time\":%d}", millis() / 1000);
        BC26MQTTPublish(topic, buff, MQTT_QOS0);
    }
}