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
#include <list>
#include <mutex>

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

class TLog;

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
    virtual void emitLastLine(String line) { return; };
    
protected:
    char * _identifier;
    TLog * _tlog = NULL;

friend TLog;
    // Small hack to allow for a single shared
    // line buffer across all writers.
    //
    void setTLog(TLog *p);
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
        if ( handlers.end() == it) {
            // we're not using push_back; that copies; but use a reference.
            // As it can see reuse.
            handlers.emplace_back(_handler);
            _handler->setTLog(this);
        };
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
        while(loopqueue.size()) {
		String line = loopqueue.front();
            	loopqueue.erase(loopqueue.begin());

        	for (auto it = handlers.begin(); it != handlers.end(); ++it) 
            		(*it)->emitLastLine(line);

		{
                 	std::lock_guard<std::mutex> lck(_historyMutex);
	        	while(queue.size() >= MAX_QUEUE_LEN)
            			queue.erase(queue.begin());
			queue.push_back(line);
		}
	};
    };
    virtual void stop() {
        for (auto it = handlers.begin(); it != handlers.end(); ++it) {
            (*it)->stop();
        }
    };
    size_t write(byte a) {
        if (lst == '\n') {
	  if (_timestamp) {
            time_t now = time(NULL);
            char buff1[30], buff2[32];
            ctime_r(&now,buff1);
	    buff1[19] = '\0';
            size_t n = snprintf(buff2,sizeof(buff2)-1, "%s.%03lu:",buff1+11,millis() % 1000);
	    buff2[n] = '\0';
            for(char * p = buff2; *p; p++)
                _dwrite(*p);
          };
          if (_identifier) {
            char buff2[32];
            size_t n = snprintf(buff2,sizeof(buff2)-1, "%s:", _identifier);
            for(char * p = buff2; *p; p++)
                _dwrite(*p);
	  };
          _dwrite(' ');
        };
        lst = a;
        return _dwrite(a);
    };

    // std::mutex historyMutex() { return _historyMutex; };
    std::mutex _historyMutex;
    std::list<String> * history() {
	return & queue;
    };

    static const int MAX_LOG_LINE = 1200;
private:
    std::vector<std::shared_ptr<LOGBase>> handlers;
    bool _disableSerial = false;
    bool _timestamp = false;
    byte lst = '\n';
    
    static const int MAX_QUEUE_LEN = 30;
    static const int MAX_LOOP_QUEUE_LEN = 7;

    std::list<String> queue, loopqueue;

    char _buff[ MAX_LOG_LINE + 5]; 
    int at = 0;

    size_t _dwrite(byte a) {
        for (auto it = handlers.begin(); it != handlers.end(); ++it) 
            (*it)->write(a);

        if (a != '\r' && a != '\n') 
		_buff[at++] = a;

        if ((a == '\n' && at) || at >= MAX_LOG_LINE) {

		// Add ellipsis on overflow
		if (a != '\n') {
			at = MAX_LOG_LINE; 
			_buff[at++] = '.';
			_buff[at++] = '.';
			_buff[at++] = '.';
                };
		_buff[at++] = '\0';
		at = 0;

//        	while(loopqueue.size() >= MAX_LOOP_QUEUE_LEN) loopqueue.erase(loopqueue.begin());
// Assum earlier log lines are more important :)
		if (loopqueue.size() < MAX_LOOP_QUEUE_LEN)
		        loopqueue.push_back(String(_buff));
	};

        if (_disableSerial)
            return 1;

        return Serial.write(a);
    } // End of _dwrite();
};

extern TLog Log, Debug;
#endif
