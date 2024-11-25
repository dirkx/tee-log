/* Copyright 2008, 2012-2022 Dirk-Willem van Gulik <dirkx(at)webweaving(dot)org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Library that provides a fanout, or T-flow; so that output or logs do
 * not just got to the serial port; but also to a configurable mix of a
 * telnetserver, a webserver, syslog or MQTT.
 */

#ifndef _H_LOG_TEE
#define _H_LOG_TEE

#include <Arduino.h>
#include <Print.h>

#include <stddef.h>
#include <memory>
#include <vector>
#include <functional>

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#define IDENTIFIER_GENERATOR (WiFi.macAddress().c_str())
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#define IDENTIFIER_GENERATOR (WiFi.macAddress().c_str())
#endif

#ifndef IDENTIFIER_GENERATOR
#define IDENTIFIER_GENERATOR "TLG"
#endif

class LOGBase : public Print {
public:
    LOGBase(const char * identifier = IDENTIFIER_GENERATOR) : _identifier(strdup(identifier)) {};
    ~LOGBase() { if (_identifier) free(_identifier); };
    String identifier() { return String(_identifier); };
    void setIdentifier(const char * identifier) { if (_identifier) free(_identifier); _identifier = strdup(identifier); };
    virtual void begin() { return; };
    virtual void reconnect() { return; };
    virtual void loop() { return; };
    virtual void stop() { return; };
    
protected:
    char * _identifier;
};

class TLog : public LOGBase
{
public:
    TLog(const char * identifier) : LOGBase(identifier) {};
    void disableSerial(bool onoff) { _disableSerial = onoff; };
    void setTimestamp(bool onoff) { _timestamp = onoff; };
    
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
        // MDNS.begin();
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
        if (_timestamp && lst == '\n') {
            
            time_t now = time(NULL);
            char buff[30], buff2[50];
            ctime_r(&now,buff);
	    buff[19] = '\0';
            snprintf(buff2,sizeof(buff2), "%s.%03lu - %s - ",buff+11,millis() % 1000, _identifier);
            for(char * p = buff2; *p; p++)
                _dwrite(*p);
        };
        lst = a;
        return _dwrite(a);
    }
private:
    std::vector<std::shared_ptr<LOGBase>> handlers;
    bool _disableSerial = false;
    bool _timestamp = false;
    byte lst = 0;
    
    size_t _dwrite(byte a) {
        for (auto it = handlers.begin(); it != handlers.end(); ++it) {
            (*it)->write(a);
        }
        if (_disableSerial)
            return 1;
        return Serial.write(a);
    }
};

extern TLog Log, Debug;
#endif
