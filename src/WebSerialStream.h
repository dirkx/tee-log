#include <WebServer.h>
#include "Log.h"

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
