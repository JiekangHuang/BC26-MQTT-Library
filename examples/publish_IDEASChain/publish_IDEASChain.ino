#include "bc26.h"

String apn         = "<YOUR APN>";
String host        = "iiot.ideaschain.com.tw";
String user        = "<YOUR ACCESS TOKEN>";
String key         = "";     // empty
String topic       = "v1/devices/me/telemetry";
String device_name = "<YOUR DEVICE NAME>";

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
    if (millis() - timer >= 10000) {
        timer = millis();
        BC26MQTTPublish(topic, "{\"boot time\":" + millis() / 1000 + "}", MQTT_QOS0);
    }
}