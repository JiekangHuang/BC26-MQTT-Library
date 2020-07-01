#include "bc26.h"

String apn   = "<YOUR APN>";
String host  = "io.adafruit.com";
String user  = "<YOUR USERNAME>";
String key   = "<YOUR ACTIVE KEY>";
String topic = "<YUOR TOPIC>";
char   msg[] = "12345";

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
    static long timer = millis();
    if (millis() - timer >= 10000) {
        timer = millis();
        BC26MQTTPublish(topic, String(timer / 1000), MQTT_QOS0);
    }
}
