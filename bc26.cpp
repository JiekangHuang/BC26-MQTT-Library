#include "bc26.h"

static String         bc26_buff;
static char           bc26_host[40], bc26_user[40], bc26_key[50];
static int            bc26_port;
static SoftwareSerial bc26(8, 9);

static bool _BC26SendCmdReply(const char *cmd, const char *reply, unsigned long timeout);

static bool _BC26SendCmdReply(const char *cmd, const char *reply, unsigned long timeout)
{
    bc26_buff = "";
    DEBUG_PRINT(cmd);
    bc26.println(cmd);
    unsigned long timer = millis();
    while (millis() - timer < timeout) {
        if (bc26.available()) {
            bc26_buff += bc26.readStringUntil('\n');
        }
        if (bc26_buff.indexOf(reply) != -1) {
            DEBUG_PRINT(bc26_buff);
            return true;
        }
    }
    return false;
}

bool BC26Init(long baudrate, const char *apn, int band)
{
    char buff[100];
    bool result = true;
    // set random seed
    randomSeed(analogRead(A0));
    // wait boot
    Serial.println(F("Wait BC26 boot....."));
    delay(5000);
    long gsm_load = 46692;
    // init nbiot SoftwareSerial
    bc26.begin(baudrate);
    // echo mode off
    result &= _BC26SendCmdReply("ATE0", "OK", 2000);
    // set band
    sprintf(buff, "AT+QBAND=1,%d", band);
    result &= _BC26SendCmdReply(buff, "OK", 2000);
    // close EDRX
    result &= _BC26SendCmdReply("AT+CEDRXS=0", "OK", 2000);
    // close SCLK
    result &= _BC26SendCmdReply("AT+QSCLK=0", "OK", 2000);

    if (!result) {
        Serial.println(F("BC26 No response ! Please Power off and power on again !!"));
        while (true)
            ;
    }

    while (!_BC26SendCmdReply("AT+CGATT?", "+CGATT: 1", 2000)) {
        if (strcmp(apn, "internet.iot") != -1) {
            gsm_load = 46692;
        } else if (strcmp(apn, "twm.nbiot") != -1) {
            gsm_load = 46697;
        } else {
            Serial.println(F("apn error !!"));
        }
        Serial.println(F("Connect to 4GAP....."));
        sprintf(buff, "AT+COPS=1,2,\"%ld\"", gsm_load);
        if (_BC26SendCmdReply(buff, "OK", 20000)) {
            Serial.println(F("Network is ok !!"));
        } else {
            Serial.println(F("Network is not ok !!"));
        }
    }
    return true;
}

bool BC26ConnectMQTTServer(const char *host, const char *user, const char *key, int port)
{
    long random_id = random(65535);
    strcpy(bc26_host, host);
    strcpy(bc26_user, user);
    strcpy(bc26_key, key);
    bc26_port = port;

    char buff[255];

    while (!_BC26SendCmdReply("AT+QMTCONN?", "+QMTCONN: 0,3", 2000)) {
        while (!_BC26SendCmdReply("AT+QMTOPEN?", "+QMTOPEN: 0,", 2000)) {
            sprintf(buff, "AT+QMTOPEN=0,\"%s\",%d", host, port);
            if (_BC26SendCmdReply(buff, "+QMTOPEN: 0,0", 20000)) {
                Serial.println(F("Opened MQTT Channel Successfully"));
            } else {
                Serial.println(F("Failed to open MQTT Channel"));
            }
        }
        sprintf(buff, "AT+QMTCONN=0,\"Arduino_BC26_%ld\",\"%s\",\"%s\"", random_id, user, key);
        if (_BC26SendCmdReply(buff, "+QMTCONN: 0,0,0", 20000)) {
        } else {
            Serial.println(F("Failed to Connect MQTT Server"));
        }
    }
    Serial.println(F("Connect MQTT Server Successfully"));
    return true;
}

bool BC26MQTTPublish(const char *topic, char *msg, int qos)
{
    char buff[200];
    long msgID = 0;
    if (qos > 0) {
        msgID = random(1, 65535);
    }
    sprintf(buff, "AT+QMTPUB=0,%ld,%d,0,\"%s\",\"%s\"", msgID, qos, topic, msg);
    DEBUG_PRINT(buff);
    while (!_BC26SendCmdReply(buff, "+QMTPUB: 0,0,0", 10000)) {
        BC26ConnectMQTTServer(bc26_host, bc26_user, bc26_key, bc26_port);
    }
    Serial.print(F("Publish :("));
    Serial.print(msg);
    Serial.println(F(") Successfully"));
    return true;
}

bool BC26MQTTSubscribe(const char *topic, int qos)
{
    char buff[200];
    sprintf(buff, "AT+QMTSUB=0,1,\"%s\",%d", topic, qos);
    while (!_BC26SendCmdReply(buff, "+QMTSUB: 0,1,0,0", 10000)) {
        BC26ConnectMQTTServer(bc26_host, bc26_user, bc26_key, bc26_port);
    }
    Serial.print(F("Subscribe Topic("));
    Serial.print(topic);
    Serial.println(F(") Successfully"));
    return true;
}

int getBC26CSQ(void)
{
    String rssi;
    int    s_idx;

    if (_BC26SendCmdReply("AT+CSQ", "+CSQ: ", 2000)) {
        s_idx = bc26_buff.indexOf("+CSQ: ");
        s_idx += 6;
        while (bc26_buff[s_idx] != ',') {
            rssi += bc26_buff[s_idx++];
        }
        return rssi.toInt();
    }
    return -1;
}

bool readBC26MQTTMsg(const char *topic, char *msg)
{
    char *head, *tail;
    char  buff[50];
    bc26_buff = "";

    if (bc26.available()) {
        bc26_buff += bc26.readStringUntil('\n');
        sprintf(buff, "+QMTRECV: 0,0,\"%s\",\"", topic);
        head = strstr(bc26_buff.c_str(), buff);
        if (head) {
            DEBUG_PRINT("receive:");
            DEBUG_PRINT(bc26_buff);
            DEBUG_PRINT(head);
            head += (15 + strlen(topic) + 3);
            DEBUG_PRINT(head);
            tail = strstr(head, "\"");
            DEBUG_PRINT(tail);
            strncpy(msg, head, tail - head);
            msg[tail - head] = '\0';
            DEBUG_PRINT(msg);
            return true;
        }
    }
    return false;
}
