#include "bc26.h"

const char *apn   = "<YOUR APN>";
const char *host  = "io.adafruit.com";
const char *user  = "<YOUR USERNAME>";
const char *key   = "<YOUR ACTIVE KEY>";
const char *topic = "<YUOR TOPIC>";
char        msg[] = "12345";

BC26 client(8, 9);

void setup()
{
    Serial.begin(BAUDRATE_38400);
    client.init(BAUDRATE_38400, BAND_8, apn);
    client.openMQTTClient(host, MQTT_PORT_1883);
    client.connectMQTTServer(user, key);
    client.subscribe(topic, MQTT_QOS0);
    client.publish(topic, msg, MQTT_QOS0);
}

void loop()
{
    static long timer = millis();
    if (millis() - timer >= 10000) {
        timer = millis();
        client.publish(topic, timer / 1000, MQTT_QOS0);
    }
}
