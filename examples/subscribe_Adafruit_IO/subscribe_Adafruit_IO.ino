#include "bc26.h"

String apn   = "<YOUR APN>";
String host  = "io.adafruit.com";
String user  = "<YOUR USERNAME>";
String key   = "<YOUR ACTIVE KEY>";
String topic = "<YUOR TOPIC>";

void setup()
{
    Serial.begin(BAUDRATE_38400);
    // bc26 init
    BC26Init(BAUDRATE_38400, apn, BAND_8);
    // connect to server
    BC26ConnectMQTTServer(host, user, key, MQTT_PORT_1883);
    // Subscribe topic
    BC26MQTTSubscribe(topic, MQTT_QOS0);
}

void loop()
{
    String message;
    if (readBC26MQTTMsg(topic, message)) {
        Serial.println(message);
    }
}
