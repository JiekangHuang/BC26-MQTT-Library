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
}

void loop()
{
    char        str[10];
    static long timer = millis();
    if (millis() - timer >= 5000) {
        timer = millis();
        if (client.getDate(str)) {
            Serial.println(str);
        }
        if (client.getTime(str)) {
            Serial.println(str);
        }
    }
}
