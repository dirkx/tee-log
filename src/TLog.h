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
#ifdef ESP32
#include <mutex>
#endif

#ifdef ESP32
#include <mutex>
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

// Ringbuffer - origin: https://github.com/AndersKaloer/Ring-Buffer
//
#include "ringbuffer/ringbuffer.h"

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
    virtual void emitLastLine(const char *line) { return; };
    // void setMaxLine(size_t max) { MAX_LOG_LINE = max; }; 
    size_t maxLine() { return MAX_LOG_LINE; };
 protected:
     char * _identifier;
     TLog * _tlog = NULL;
     static const size_t MAX_LOG_LINE = 512;

friend TLog;
    // Small hack to allow for a single shared
    // line buffer across all writers.
    //
    void setTLog(TLog *p);

};

class TLog : public LOGBase
{
public:
    TLog(): TLog(IDENTIFIER_GENERATOR) {};
    TLog(const char * identifier) : LOGBase(identifier) {
        assert(MAX_LOG_LINE < sizeof(queue_buff));
        assert(MAX_LOG_LINE < sizeof(loopqueue_buff));
        assert(sizeof(queue_buff) >=  sizeof(loopqueue_buff));
	ring_buffer_init(&queue, queue_buff, sizeof(queue_buff));
	ring_buffer_init(&loopqueue, loopqueue_buff, sizeof(loopqueue_buff));
    };

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
        while(!ring_buffer_is_empty(&loopqueue)) {
		char line[MAX_LOG_LINE], *p = line;
		while(ring_buffer_dequeue(&loopqueue, p) && *p) p++;
		size_t len = p - line;

        	for (auto it = handlers.begin(); it != handlers.end(); ++it) 
            		(*it)->emitLastLine(line);

		{
#ifdef ESP32
                 	std::lock_guard<std::mutex> lck(_historyMutex);
#endif
			// remove complete lines if there is no space. Lines
			// are always shorter than the whole buffer.
			while(ring_buffer_num_items(&queue) + len + 1 > sizeof(queue_buff)) {
				char p;
		                while(ring_buffer_dequeue(&queue, &p) && p) {};
			};
			ring_buffer_queue_arr(&queue, line, strlen(line) + 1);
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
            snprintf(buff2,sizeof(buff2)-1, "%s:", _identifier);
            for(char * p = buff2; *p; p++)
                _dwrite(*p);
	  };
          _dwrite(' ');
        };
        lst = a;
        return _dwrite(a);
    };

    size_t getNumberOfHistoryLines() {
	size_t i = 0, n = 0;
	char p;
	while(ring_buffer_peek(&queue, &p, i++)) {
		if (p == '\0')
			n++;
	};
	return n;
    };

    bool getHistoryLine(size_t age, char * buff) {
#ifdef ESP32
	std::lock_guard<std::mutex> lck(_historyMutex);
#endif
	size_t i = 0;
	char * p = buff;

	for(;1;) {
		if (!ring_buffer_peek(&queue, p, i++)) {
			// ran out of buffer before end of line or we are at
			// the end of a filled buffer with no more lines
			// left before the head.
			return false; 
		};
		if (*p == '\0') {
			if (!age) {
				return true;
			};
			p = buff;
			age --;
		} else  {
			p++;
		};
	};
        return false;
    };

private:
    std::vector<std::shared_ptr<LOGBase>> handlers;
    bool _disableSerial = false;
    bool _timestamp = false;
    byte lst = '\n';
    
    ring_buffer_t queue, loopqueue;
    char queue_buff[ 2 * 1024 ]; // 4k gets a bit big for the Async socket.
    char loopqueue_buff[ 1 * 1024]; 

    char _buff[ MAX_LOG_LINE ];
    int at = 0;

#ifdef ESP32
    // std::mutex historyMutex() { return _historyMutex; };
    std::mutex _historyMutex;
#endif

    size_t _dwrite(byte a) {
        for (auto it = handlers.begin(); it != handlers.end(); ++it) 
            (*it)->write(a);

        if (a != '\r' && a != '\n') 
		_buff[at++] = a;

        if ((a == '\n' && at) || (at >= MAX_LOG_LINE-2)) {
		// Add ellipsis on overflow
		if (a != '\n') {
			at = MAX_LOG_LINE-5;
			_buff[at++] = '.';
			_buff[at++] = '.';
			_buff[at++] = '.';
                };
		_buff[at++] = '\0';

                // remove complete lines if there is no space. Lines
                // are always shorter than the whole buffer. Not sure
                // if this is the right strategy - we could also
                // at this point stop adding lines to the queue; so that
                // the earlier ??errors?? show.
                while(ring_buffer_num_items(&loopqueue) + at > sizeof(loopqueue_buff)) {
		      char p;
                      while(ring_buffer_dequeue(&loopqueue, &p) && p) {};
		};
                ring_buffer_queue_arr(&loopqueue, _buff, at);
		at = 0;
	};

        if (_disableSerial)
            return 1;

        return Serial.write(a);
    } // End of _dwrite();
};

extern TLog Log, Debug;
#endif
