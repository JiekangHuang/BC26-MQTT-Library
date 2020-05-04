#ifndef BC26_H
#define BC26_H
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>

#define DEBUG_MODE false
#if DEBUG_MODE
#define debugSerial Serial
#define DEBUG_PRINT(x) debugSerial.println(x)
#else
#define DEBUG_PRINT(x)
#endif

#define BAUDRATE_9600 9600
#define BAUDRATE_19200 19200
#define BAUDRATE_38400 38400
#define BAUDRATE_115200 115200

#define RX_BUFFSIZE 255
#define TRY_COUNT_MAX 10

#define MQTT_PORT_1883 1883
#define MQTT_PORT_8883 8883

typedef enum
{
    MQTT_INIT = 1,
    MQTT_CONNECTING,
    MQTT_CONNECTED,
    MQTT_DISCONNECTING,
} BC26_CONNECT_STATE_E;

typedef enum
{
    ATE0,
    AT_CGATT,
    AT_CEDRXS_0,
} BC26_SEND_CMD_E;

typedef enum
{
    OK,
} BC26_RECEIVE_CMD_E;

typedef enum
{
    MQTT_QOS0,
    MQTT_QOS1,
} MQTT_QOS_E;

typedef enum
{
    BAND_1  = 1,
    BAND_3  = 3,
    BAND_8  = 8,
    BAND_28 = 28,
} BC26_BAND;

class BC26 : public SoftwareSerial {
  private:
    uint8_t  mqtt_client_id;
    char     mqtt_host[20];
    uint16_t mqtt_port;
    char     username[50];
    char     key[50];
    char     buff[RX_BUFFSIZE];

    void rx_data(void);
    void clearSerialBuff(void);
    bool sendCommandOnly(const char *cmd);
    bool sendCommandReply(const char *cmd, const char *reply, uint32_t timeout);
    bool echoModeOff(void);
    bool chkNetwork(void);
    bool setBand(uint8_t band);
    bool setApn(const char *apn);
    bool setCops(const char *apn);
    bool chkMQTTOpen(void);
    bool chkMQTTConn(void);
    bool cloeEDRX(void);

  public:
    BC26(uint8_t rx_pin, uint8_t tx_pin);
    ~BC26();
    int  init(uint32_t baudrate, uint8_t band, const char *apn);
    bool openMQTTClient(const char *host, uint16_t port = 1883);
    bool closeMQTTClient(void);
    bool connectMQTTServer(const char *user, const char *key);
    bool publish(const char *topic, const char *msg, uint8_t qos = 0);
    bool publish(const char *topic, long msg, uint8_t qos = 0);
    bool publish(const char *topic, double msg, uint8_t qos = 0);
    bool subscribe(const char *topic, uint8_t qos = 0);
    bool readMsg(char *topic, char *msg);
};

#endif /* BC26_H */