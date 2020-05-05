#include "bc26.h"

static const char *send_cmd[] = {
     "ATE0", "AT+CGATT?", "AT+CEDRXS=0", "AT+QSCLK=0", "AT+QCCLK?",
};

static const char *receive_cmd[] = {
     "OK",
};

BC26::BC26(uint8_t rx_pin, uint8_t tx_pin) : SoftwareSerial(rx_pin, tx_pin)
{
}

BC26::~BC26()
{
}

int BC26::init(uint32_t baudrate, uint8_t band, const char *apn)
{
    if (apn == NULL) {
        return false;
    }
    if (baudrate != BAUDRATE_9600 && baudrate != BAUDRATE_19200 && baudrate != BAUDRATE_38400) {
        return false;
    }
    randomSeed(A0);
    uint8_t try_count = 0;
    memset(this->buff, '\0', RX_BUFFSIZE);
    this->begin(baudrate);
    while (try_count++ < TRY_COUNT_MAX && !this->chkNetwork()) {
        if (this->echoModeOff()) {
            if (this->setBand(band)) {
                if (this->setApn(apn)) {
                    if (this->setCops(apn)) {
                        DEBUG_PRINT(F("BC26 NetWork OK !!"));
                        return true;
                    }
                }
            }
        }
        if (try_count >= TRY_COUNT_MAX) {
            DEBUG_PRINT(F("BC26 NetWork Error !!"));
        } else {
            DEBUG_PRINT(F("BC26 NetWork OK !!"));
            return true;
        }
    }
    return false;
}

bool BC26::sendCommandOnly(const char *cmd)
{
    if (cmd == NULL) {
        return false;
    }
    clearSerialBuff();
    this->println(cmd);
    DEBUG_PRINT(cmd);
    return true;
}

bool BC26::sendCommandReply(const char *cmd, const char *reply, uint32_t timeout)
{
    if (cmd == NULL) {
        return false;
    }
    memset(this->buff, '\0', RX_BUFFSIZE);
    clearSerialBuff();
    this->println(cmd);
    DEBUG_PRINT(cmd);
    uint32_t timer = millis();
    while (millis() - timer < timeout) {
        this->rx_data();
        if (strstr(this->buff, reply)) {
            return true;
        }
    }
    return false;
}

void BC26::rx_data(void)
{
    if (this->available()) {
        strcat(this->buff, this->readStringUntil('\n').c_str());
        // DEBUG_PRINT(this->buff);
    }
}

void BC26::clearSerialBuff(void)
{
    while (this->available()) {
        this->read();
    }
}

bool BC26::echoModeOff(void)
{
    return this->sendCommandReply(send_cmd[ATE0], receive_cmd[OK], 1000);
}

bool BC26::chkNetwork(void)
{
    if (this->sendCommandReply(send_cmd[AT_CGATT], receive_cmd[OK], 1000)) {
        if (strstr(this->buff, "+CGATT: 0")) {
            DEBUG_PRINT(F("Net is not ok !!"));
            return false;
        } else if (strstr(this->buff, "+CGATT: 1")) {
            DEBUG_PRINT(F("Net is ok !!"));
            return true;
        }
    }
    return false;
}

bool BC26::setBand(uint8_t band)
{
    char temp_cmd[50];
    sprintf(temp_cmd, "AT+QBAND=1,%d", band);
    return this->sendCommandReply(temp_cmd, receive_cmd[OK], 2000);
}

bool BC26::setApn(const char *apn)
{
    char temp_cmd[50];
    sprintf(temp_cmd, "AT+QCGDEFCONT=\"IP\",\"%s\"", apn);
    return this->sendCommandReply(temp_cmd, receive_cmd[OK], 2000);
}

bool BC26::setCops(const char *apn)
{
    char temp_cmd[50];
    long gsm_load = 0;
    if (!strcmp(apn, "internet.iot")) {
        gsm_load = 46692;
    } else if (!strcmp(apn, "twm.nbiot")) {
        gsm_load = 46697;
    }
    if (gsm_load != 0) {
        sprintf(temp_cmd, "AT+COPS=1,2,\"%ld\"", gsm_load);
        return this->sendCommandReply(temp_cmd, receive_cmd[OK], 2000);
    }
    return false;
}

bool BC26::openMQTTClient(const char *host, uint16_t port)
{
    if (host == NULL) {
        return false;
    }
    if (port != MQTT_PORT_1883) {
        return false;
    }
    char temp_cmd[50], temp_reply[20];
    int  client_id = -1, result, try_count = 0;
    strcpy(this->mqtt_host, host);
    this->mqtt_port = port;
    sprintf(temp_cmd, "AT+QMTOPEN=%d,\"%s\",%d", this->mqtt_client_id, this->mqtt_host,
            this->mqtt_port);
    sprintf(temp_reply, "+QMTOPEN: %d,", this->mqtt_client_id);
    while (try_count++ < TRY_COUNT_MAX) {
        this->closeMQTTClient();
        if (this->sendCommandReply(temp_cmd, temp_reply, 10000)) {
            char *find = strstr(this->buff, temp_reply);
            if (find) {
                sscanf_P(find, PSTR("+QMTOPEN: %d,%d"), &client_id, &result);
                if (client_id == this->mqtt_client_id)
                    switch (result) {
                    default:
                    case -1:
                        DEBUG_PRINT(F("Failed to open network"));
                    case 0:
                        DEBUG_PRINT(F("Opened network successfully"));
                        return true;
                        break;
                    case 1:
                        DEBUG_PRINT(F("Wrong parameter"));
                        break;
                    case 2:
                        DEBUG_PRINT(F("MQTT identifier is occupied"));
                        break;
                    case 3:
                        DEBUG_PRINT(F("Failed to activate PDP"));
                        break;
                    case 4:
                        DEBUG_PRINT(F("Failed to parse domain name"));
                        break;
                    case 5:
                        DEBUG_PRINT(F("Network disconnection error"));
                        break;
                    }
            }
        }
    }
    return false;
}

bool BC26::chkMQTTOpen(void)
{
    char temp_cmd[] = "AT+QMTOPEN?", temp_reply[20];
    int  try_count  = 0;
    while (try_count++ < TRY_COUNT_MAX) {
        if (this->sendCommandReply(temp_cmd, receive_cmd[OK], 2000)) {
            sprintf(temp_reply, "+QMTOPEN: %d,", this->mqtt_client_id);
            char *find = strstr(this->buff, temp_reply);
            if (find) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

bool BC26::closeMQTTClient(void)
{
    char temp_cmd[50], temp_reply[20];
    int  client_id = -1, result = -1, try_count = 0;
    sprintf(temp_cmd, "AT+QMTCLOSE=%d", this->mqtt_client_id);
    sprintf(temp_reply, "+QMTCLOSE: %d,", this->mqtt_client_id);
    while (try_count++ < TRY_COUNT_MAX && this->chkMQTTOpen()) {
        if (this->sendCommandReply(temp_cmd, temp_reply, 2000)) {
            char *find = strstr(this->buff, temp_reply);
            if (find) {
                sscanf_P(find, PSTR("+QMTCLOSE: %d,%d"), &client_id, &result);
                if (client_id == this->mqtt_client_id) {
                    switch (result) {
                    default:
                    case -1:
                        DEBUG_PRINT(F("Failed to close network"));
                        break;
                    case 0:
                        DEBUG_PRINT(F("Network close successfully"));
                        return true;
                        break;
                    }
                }
            }
        }
    }
    return false;
}

bool BC26::chkMQTTConn(void)
{
    char temp_cmd[] = "AT+QMTCONN?", temp_reply[20];
    int  try_count  = 0;
    while (try_count++ < TRY_COUNT_MAX) {
        if (this->sendCommandReply(temp_cmd, receive_cmd[OK], 2000)) {
            sprintf(temp_reply, "+QMTCONN: %d,%d", this->mqtt_client_id, MQTT_CONNECTED);
            char *find = strstr(this->buff, temp_reply);
            if (find) {
                DEBUG_PRINT(F("BC26 MQTT CONNECTED !!"));
                return true;
            } else {
                return false;
            }
            DEBUG_PRINT("here");
        }
    }
    return false;
}

bool BC26::connectMQTTServer(const char *user, const char *key)
{
    if (user == NULL || key == NULL) {
        return false;
    }
    char temp_cmd[200], temp_reply[20];
    long random_id = random(65535);
    int  client_id = -1, result, ret_code, try_count = 0;
    strcpy(this->username, user);
    strcpy(this->key, key);
    sprintf(temp_reply, "+QMTCONN: %d,", this->mqtt_client_id);
    sprintf(temp_cmd, "AT+QMTCONN=%d,\"Arduino_BC26_%ld\",\"%s\",\"%s\"", this->mqtt_client_id,
            random_id, this->username, this->key);
    while (try_count++ < TRY_COUNT_MAX) {
        if (!this->chkMQTTConn()) {
            if (this->sendCommandReply(temp_cmd, temp_reply, 10000)) {
                char *find = strstr(this->buff, temp_reply);
                if (find) {
                    sscanf_P(find, PSTR("+QMTCONN: %d,%d,%d"), &client_id, &result, &ret_code);
                    if (client_id == this->mqtt_client_id) {
                        switch (result) {
                        case 0:
                            DEBUG_PRINT(F("Sent packet successfully and received ACK fromserver"));
                            break;
                        case 1:
                            DEBUG_PRINT(F("Packet retransmission"));
                            break;
                        case 2:
                            DEBUG_PRINT(F("Failed to send packet"));
                            break;
                        default:
                            break;
                        }
                        switch (ret_code) {
                        case 0:
                            DEBUG_PRINT(F("Connection Accepted"));
                            break;
                        case 1:
                            DEBUG_PRINT(F("Connection Refused: Unacceptable Protocol Version"));
                            break;
                        case 2:
                            DEBUG_PRINT(F("Connection Refused: Identifier Rejected"));
                            break;
                        case 3:
                            DEBUG_PRINT(F("Connection Refused: Server Unavailable"));
                            break;
                        case 4:
                            DEBUG_PRINT(F("Connection Refused: Bad User Name or Password"));
                            break;
                        case 5:
                            DEBUG_PRINT(F("Connection Refused: Not Authorized"));
                            break;
                        default:
                            break;
                        }
                        if (!(result | ret_code)) {
                            return true;
                        }
                    }
                }
            }
        } else {
            DEBUG_PRINT(F("Connection Accepted"));
            return true;
        }
    }
    return false;
}

bool BC26::publish(const char *topic, const char *msg, uint8_t qos)
{
    if (topic == NULL || msg == NULL) {
        return false;
    }
    if (qos != MQTT_QOS0 && qos != MQTT_QOS1) {
        qos = MQTT_QOS0;
    }
    char temp_cmd[200], temp_reply[20];
    long msgID = 0, temp_msgID = -1;
    int  client_id = -1, result = -1, try_count = 0;
    if (qos >= MQTT_QOS1) {
        msgID = random(1, 65535);
    }
    while (try_count++ < TRY_COUNT_MAX) {
        if (!this->chkMQTTConn()) {
            this->connectMQTTServer(this->username, this->key);
        }
        sprintf(temp_reply, "+QMTPUB: %d,", this->mqtt_client_id);
        sprintf(temp_cmd, "AT+QMTPUB=%d,%ld,%d,0,\"%s\",\"%s\"", this->mqtt_client_id, msgID, qos,
                topic, msg);
        if (this->sendCommandReply(temp_cmd, temp_reply, 10000)) {
            char *find = strstr(this->buff, temp_reply);
            if (find) {
                sscanf_P(find, PSTR("+QMTPUB: %d,%ld,%d"), &client_id, &temp_msgID, &result);
                if (client_id == this->mqtt_client_id && temp_msgID == msgID) {
                    switch (result) {
                    case 0:
                        DEBUG_PRINT(F("Sent packet successfully and received ACK from server"));
                        DEBUG_PRINT(F("BC26 PUBLISH SUCCESS !!"));
                        return true;
                        break;
                    case 1:
                        DEBUG_PRINT(F("Packet retransmission"));
                        break;
                    case 2:
                        DEBUG_PRINT(F("Failed to send packet"));
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    DEBUG_PRINT(F("BC26 PUBLISH Failed !!"));
    return false;
}

bool BC26::publish(const char *topic, int msg, uint8_t qos)
{
    return this->publish(topic, (long)msg, qos);
}

bool BC26::publish(const char *topic, long msg, uint8_t qos)
{
    char        temp_msg[20];
    const char *ptr = temp_msg;
    sprintf(temp_msg, "%ld", msg);
    return this->publish(topic, ptr, qos);
}

bool BC26::publish(const char *topic, double msg, uint8_t qos)
{
    return this->publish(topic, String(msg, 3).c_str(), qos);
}

bool BC26::publish(const char *topic, String msg, uint8_t qos)
{
    return this->publish(topic, msg.c_str(), qos);
}

bool BC26::subscribe(const char *topic, uint8_t qos)
{
    if (topic == NULL) {
        return false;
    }
    if (qos != MQTT_QOS0 && qos != MQTT_QOS1) {
        qos = MQTT_QOS0;
    }
    char temp_cmd[200], temp_reply[20];
    long msgID = random(1, 65535), temp_msgID = -1;
    int  client_id = -1, result = -1, temp_qos = -1, try_count = 0;
    while (try_count++ < TRY_COUNT_MAX && this->chkMQTTConn()) {
        sprintf(temp_reply, "+QMTSUB: %d,", this->mqtt_client_id);
        sprintf(temp_cmd, "AT+QMTSUB=%d,%ld,\"%s\",%d", this->mqtt_client_id, msgID, topic, qos);
        if (this->cloeEDRX() && this->closeSCLK()) {
            if (this->sendCommandReply(temp_cmd, temp_reply, 5000)) {
                char *find = strstr(this->buff, temp_reply);
                if (find) {
                    sscanf_P(find, PSTR("+QMTSUB: %d,%ld,%d,%d"), &client_id, &temp_msgID, &result,
                             &temp_qos);
                    if (client_id == this->mqtt_client_id && temp_msgID == msgID &&
                        temp_qos == qos) {
                        switch (result) {
                        case 0:
                            DEBUG_PRINT(F("Sent packet successfully and received ACK from server"));
                            DEBUG_PRINT(F("BC26 SUNSCRIBE SUCCESS !!"));
                            return true;
                            break;
                        case 1:
                            DEBUG_PRINT(F("Packet retransmission"));
                            break;
                        case 2:
                            DEBUG_PRINT(F("Failed to send packet"));
                            break;
                        default:
                            break;
                        }
                    } else if (temp_qos == 128) {
                        DEBUG_PRINT(F("subscription was rejected by the server"));
                        return false;
                    }
                }
            }
        }
    }
    DEBUG_PRINT(F("BC26 SUNSCRIBE Failed !!"));
    return false;
}

bool BC26::cloeEDRX(void)
{
    return this->sendCommandReply(send_cmd[AT_CEDRXS_0], receive_cmd[OK], 1000);
}

bool BC26::closeSCLK(void)
{
    return this->sendCommandReply(send_cmd[AT_QSCLK_0], receive_cmd[OK], 1000);
}

bool BC26::readMsg(char *topic, char *msg)
{
    if (topic == NULL || msg == NULL) {
        return false;
    }
    char temp_reply[20];
    int  client_id = -1;
    this->rx_data();
    sprintf(temp_reply, "+QMTRECV: %d,", this->mqtt_client_id);
    char *find = strstr(this->buff, temp_reply);
    if (find) {
        sscanf_P(find, PSTR("+QMTRECV: %d,0,\"%[^\",]\",\"%[^\"]"), &client_id, topic, msg);
        memset(this->buff, '\0', RX_BUFFSIZE);
        if (client_id == this->mqtt_client_id) {
            return true;
        }
    }
    return false;
}

bool BC26::getDate(char *buf)
{
    if (this->sendCommandReply(send_cmd[AT_QCCLK], receive_cmd[OK], 1000)) {
        char *find = strstr(this->buff, "+QCCLK: ");
        if (find) {
            sscanf_P(find, PSTR("+QCCLK: %[^,],%*[^]"), buf);
            memset(this->buff, '\0', RX_BUFFSIZE);
            return true;
        }
    }
    return false;
}

bool BC26::getTime(char *buf)
{
    if (this->sendCommandReply(send_cmd[AT_QCCLK], receive_cmd[OK], 1000)) {
        char *find = strstr(this->buff, "+QCCLK: ");
        if (find) {
            sscanf_P(find, PSTR("+QCCLK: %*[^,],%[^+]"), buf);
            memset(this->buff, '\0', RX_BUFFSIZE);
            return true;
        }
    }
    return false;
}
