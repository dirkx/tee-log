#ifndef _H_MqttStream
#define _H_MqttStream

#include <Print.h>
#include <PubSubClient.h>

#include "Log.h"


class MqttStream : public TLog {
  public:
    MqttStream(Client * client, const char * mqttServer = NULL, const char * mqttTopic = NULL, const uint16_t mqttPort = 1883) :
      _client(client), _mqttServer(mqttServer), _mqttTopic(mqttTopic), _mqttPort(mqttPort) {
    };

    void setPort(uint16_t port) {
      _mqttPort = port;
    }

    void setTopic(const char * topic) {
      if (topic)
        _mqttTopic = strdup(topic);
    }

    void setServer(const char * server) {
      if (server)
        _mqttServer = strdup(server);
    }

    virtual size_t write(uint8_t c);
    virtual void begin();
    virtual void loop();

  private:
    Client * _client;
    PubSubClient * _mqtt = NULL;
    const char * _mqttServer, * _mqttTopic;
    uint16_t _mqttPort;
    char logbuff[512];
    size_t at = 0;
  protected:
};
#endif
