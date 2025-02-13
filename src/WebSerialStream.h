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

#ifndef _H_WEBSERVER_TEE_LOG
#define _H_WEBSERVER_TEE_LOG
#if (defined(ESP32) || defined(ESP8266))

#include <TLog.h>

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#endif

class AsyncWebSocketWithData;
class WebSerialStream : public LOGBase {
  public:
    WebSerialStream(const uint16_t webPort = 80) : 
		_webPort(webPort) { Serial.printf("WSS create %p\n", this); };
    WebSerialStream(AsyncWebServer * webServer, String urlPrefix ) 
		: _server(webServer), _prefix(urlPrefix) {  Serial.printf("WSS create %p\n", this); };
    ~WebSerialStream();

    virtual void begin();
    virtual void loop();
    virtual void stop();

    virtual void emitLastLine(String s); 
    virtual size_t write(uint8_t c);
  private:
    bool _intSrv = false;
    uint16_t _webPort = 80;
    AsyncWebServer * _server = NULL;
    AsyncWebSocketWithData * _ws = NULL;
    String _prefix = ""; // i.e. the /
  protected:
};
#endif
#endif

