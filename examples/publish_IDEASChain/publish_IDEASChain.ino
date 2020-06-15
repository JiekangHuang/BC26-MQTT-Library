#include "bc26.h"

const char *apn         = "<YOUR APN>";
const char *host        = "iiot.ideaschain.com.tw";
const char *user        = "<YOUR ACCESS TOKEN>";
const char *key         = "";     // empty
const char *topic       = "v1/devices/me/telemetry";
const char *device_name = "<YOUR DEVICE NAME>";

BC26 client(8, 9);

void pubIDEASChain(const char *device_name, const char *value);

void setup()
{
    Serial.begin(BAUDRATE_38400);
    client.init(BAUDRATE_38400, BAND_8, apn);
    client.openMQTTClient(host, MQTT_PORT_1883);
    client.connectMQTTServer(user, key);
}

void loop()
{
    static unsigned long timer = millis();
    char                 buff[20];
    if (millis() - timer >= 10000) {
        timer = millis();
        sprintf(buff, "%ld", timer / 1000);
        pubIDEASChain(device_name, buff);
    }
}

void pubIDEASChain(const char *device_name, const char *value)
{
    char buff[50];
    sprintf(buff, "{\"%s\":\"%s\"}", device_name, value);
    client.publish(topic, buff, MQTT_QOS0);
}
