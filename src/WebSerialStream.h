// Simple 'tee' class - that sends all 'serial' port data also to the TelnetSerial and/or MQTT bus -
// to the 'log' topic if such is possible/enabled.
//
// XXX should refactor in a generic buffered 'add a Stream' class and then
// make the various destinations classes in their own right you can 'add' to the T.
//
//
#include "Log.h"
#include <WebServer.h>

class WebSerialStream : public TLog {
  public:
    WebSerialStream(const uint16_t webPort = 80) : _webPort(webPort) {};
    ~WebSerialStream();
    virtual size_t write(uint8_t c);
    virtual void begin();
    virtual void loop();
    virtual void stop();

  private:
    uint16_t _webPort;
    WebServer * _server = NULL;
    uint8_t _buff[1024];
    int _at = 0;
  protected:
};
