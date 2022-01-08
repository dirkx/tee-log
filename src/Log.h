#ifndef _H_LOG_TEE
#define _H_LOG_TEE

#include <stddef.h>
#include <memory>
#include <vector>
#include <functional>

#include <WiFi.h>

#define REPORT_INTERVAL (10*60*1000)

class LOGBase : public Print {
public:
    LOGBase(String identifier = WiFi.macAddress()) : _identifier(identifier) {};
    String identifier() { return _identifier; };
    void setIdentifier(String identifier) { _identifier = identifier; };
    virtual void begin() { return; };
    virtual void loop() { return; };
    virtual void stop() { return; };

protected:
    String _identifier;
};

class TLog : public LOGBase
{
  public:
    void disableSerial(bool onoff) { _disableSerial = onoff; };
    //void addPrintStream2(const LOGBase * _handler) { addPrintStream(std::make_shared<LOGBase>(_handler)); }
    void addPrintStream(const std::shared_ptr<LOGBase> &_handler) {
      auto it = find(handlers.begin(), handlers.end(), _handler);
      if ( handlers.end() == it)
        // we're not using push_back; that copies; but use a reference. 
        // As it can see reuse.
        handlers.emplace_back(_handler); 
    };
    virtual void begin() {
      for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        (*it)->begin();
      }
    };
    virtual void loop() {
      for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        (*it)->loop();
      }
    };
    virtual void stop() {
      for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        (*it)->stop();
      }
    };
    size_t write(byte a) {
      for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        (*it)->write(a);
      }
      if (_disableSerial)
          return 1;
      return Serial.write(a);
    }
  private:
    std::vector<std::shared_ptr<LOGBase>> handlers;
    bool _disableSerial = false;
};

extern TLog Log;
extern TLog Debug;
#endif
