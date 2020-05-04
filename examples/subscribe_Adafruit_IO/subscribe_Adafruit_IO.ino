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
    char topic[50], msg[20];
    if (client.readMsg(topic, msg)) {
        Serial.print(F("Topic = "));
        Serial.println(topic);
        Serial.print(F("Massage = "));
        Serial.println(msg);
    }
}
