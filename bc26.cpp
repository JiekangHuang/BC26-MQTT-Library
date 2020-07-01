#include "bc26.h"

static String         bc26_buff, bc26_host, bc26_user, bc26_key;
static int            bc26_port;
static SoftwareSerial bc26(8, 9);

static bool _nbiotSendCommandReply(String cmd, String reply, unsigned long timeout);

static bool _nbiotSendCommandReply(String cmd, String reply, unsigned long timeout)
{
    bc26_buff = "";
    bc26.println(cmd);
    Serial.println(cmd);
    unsigned long timer = millis();
    while (millis() - timer < timeout) {
        if (bc26.available()) {
            bc26_buff += bc26.readStringUntil('\n');
            // Serial.println(bc26_buff);
        }
        if (bc26_buff.indexOf(reply) != -1) {
            return true;
        }
    }
    return false;
}

bool BC26Init(long baudrate, String apn, int band)
{
    // set random seed
    randomSeed(analogRead(A0));
    // wait boot
    delay(5000);
    long gsm_load = 46692;
    // init nbiot SoftwareSerial
    bc26.begin(baudrate);
    // echo mode off
    _nbiotSendCommandReply("ATE0", "OK", 2000);
    // set band
    _nbiotSendCommandReply("AT+QBAND=1," + String(band), "OK", 2000);
    // close EDRX
    _nbiotSendCommandReply("AT+CEDRXS=0", "OK", 2000);
    // close SCLK
    _nbiotSendCommandReply("AT+QSCLK=0", "OK", 2000);

    while (!_nbiotSendCommandReply("AT+CGATT?", "+CGATT: 1", 2000)) {
        if (apn == "internet.iot") {
            gsm_load = 46692;
        } else if (apn == "twm.nbiot") {
            gsm_load = 46697;
        }
        if (_nbiotSendCommandReply("AT+COPS=1,2,\"" + String(gsm_load, DEC) + "\"", "OK", 20000)) {
            Serial.println(F("Network is ok !!"));
        } else {
            Serial.println(F("Network is not ok !!"));
        }
    }
    return true;
}

bool BC26ConnectMQTTServer(String host, String user, String key, int port)
{
    long random_id = random(65535);
    bc26_host      = host;
    bc26_user      = user;
    bc26_key       = key;
    bc26_port      = port;

    while (!_nbiotSendCommandReply("AT+QMTCONN?", "+QMTCONN: 0,3", 2000)) {
        while (!_nbiotSendCommandReply("AT+QMTOPEN?", "+QMTOPEN: 0,", 2000)) {
            if (_nbiotSendCommandReply("AT+QMTOPEN=0,\"" + host + "\"," + String(port, DEC),
                                       "+QMTOPEN: 0,0", 20000)) {
                Serial.println(F("Opened MQTT Channel Successfully"));
            } else {
                Serial.println(F("Failed to open MQTT Channel"));
            }
        }
        if (_nbiotSendCommandReply("AT+QMTCONN=0,\"Arduino_BC26_" + String(random_id, DEC) +
                                        "\",\"" + user + "\",\"" + key + "\"",
                                   "+QMTCONN: 0,0,0", 20000)) {
            Serial.println(F("Connect MQTT Server Successfully"));
        } else {
            Serial.println(F("Failed to Connect MQTT Server"));
        }
    }
    return true;
}

bool BC26MQTTPublish(String topic, String msg, int qos)
{
    long msgID = 0;
    if (qos > 0) {
        msgID = random(1, 65535);
    }
    while (!_nbiotSendCommandReply("AT+QMTPUB=0," + String(msgID, DEC) + "," + String(qos, DEC) +
                                        ",0,\"" + topic + "\",\"" + msg + "\"",
                                   "+QMTPUB: 0," + String(msgID, DEC) + ",0", 10000)) {
        BC26ConnectMQTTServer(bc26_host, bc26_user, bc26_key, bc26_port);
    }
    return true;
}

bool BC26MQTTSubscribe(String topic, int qos)
{
    long msgID = random(1, 65535);
    while (!_nbiotSendCommandReply("AT+QMTSUB=0," + String(msgID, DEC) + ",\"" + topic + "\"," +
                                        String(qos, DEC),
                                   "+QMTSUB: 0," + String(msgID, DEC) + ",0,0", 10000)) {
        BC26ConnectMQTTServer(bc26_host, bc26_user, bc26_key, bc26_port);
    }
    return true;
}

int getBC26CSQ(void)
{
    String rssi;
    int    s_idx;
    if (_nbiotSendCommandReply("AT+CSQ", "+CSQ: ", 2000)) {

        s_idx = bc26_buff.indexOf("+CSQ: ");
        s_idx += 6;
        while (bc26_buff[s_idx] != ',') {
            rssi += bc26_buff[s_idx++];
        }
        return rssi.toInt();
    }
    return -1;
}

bool readBC26MQTTMsg(String topic, String &msg)
{
    int           s_idx;
    unsigned long timer;
    bc26_buff = msg = "";

    if (bc26.available()) {
        bc26_buff += bc26.readStringUntil('\n');
        // Serial.println("receive:");
        // Serial.println(bc26_buff);
        s_idx = bc26_buff.indexOf("+QMTRECV: 0,0,\"" + topic + "\",\"");
        if (s_idx != -1) {
            s_idx += (15 + topic.length() + 3);
            timer = millis() + 100;
            while (bc26_buff[s_idx] != '\"' && millis() < timer) {
                // Serial.print(bc26_buff[s_idx++]);
                msg += bc26_buff[s_idx++];
            }
        }
        return true;
    }
    return false;
}
