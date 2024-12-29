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

#include "TLog.h"
#include "WebSerialStream.h"
#include "WebSerialStreamPage.h"

#if (defined(ESP32) || defined(ESP8266))

class AsyncWebSocketWithData : public AsyncWebSocket {
	public:	
		AsyncWebSocketWithData(String str) : AsyncWebSocket(str) {};
		void setData(void *d) { _d = d; };
	   	void *data() { return _d; };
        private:
		void * _d = NULL;
};


void WebSerialStream::emitLastLine(String line) {
	if (_ws && _ws->count())
		_ws->textAll(line);
};

size_t WebSerialStream::write(uint8_t c) {
  return 1;
}

WebSerialStream::~WebSerialStream() {
  stop();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  WebSerialStream * me = (WebSerialStream*)(((AsyncWebSocketWithData*)server)->data());

  switch (type) {
    case WS_EVT_CONNECT:
      break;
    case WS_EVT_DISCONNECT:
      break;
    case WS_EVT_DATA: {
	  AwsFrameInfo *info = (AwsFrameInfo*)arg;
	  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
		data[len] = 0;
		if (strcmp((char*)data, "getHistory") == 0) {
			client->text(me->history());
		}
	  }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void WebSerialStream::begin() {
  if (_server == NULL) {
  	_server = new AsyncWebServer(_webPort);
	_intSrv = true;
   };

   if (_ws == NULL) 
	_ws = new AsyncWebSocketWithData(_prefix + "/ws");

  _ws->setData(this);
#if 1
  _ws->onEvent(onEvent);
#else
  _ws->onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  WebSerialStream * me = (WebSerialStream*)(((AsyncWebSocketWithData*)server)->data());

  switch (type) {
    case WS_EVT_CONNECT:
      me->_connected++;
      break;
    case WS_EVT_DISCONNECT:
      me->_connected--;
      break;
    case WS_EVT_DATA: {
          AwsFrameInfo *info = (AwsFrameInfo*)arg;
          if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                data[len] = 0;
                if (strcmp((char*)data, "getHistory") == 0) {
                        client->text(me->history());
                }
          }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
});
#endif

  _server->addHandler(_ws);

  _server->on(String(_prefix).c_str(), HTTP_GET, [this](AsyncWebServerRequest *request) {
     size_t len = strlen(page) + _prefix.length() + 3 + 1 + 1;;
     char * buff = (char *)malloc(len);
     len = snprintf(buff,len-1,page,String(_prefix+"/ws").c_str());
     buff[len] = '\0';
     request->send(200, "text/html", String(buff));
  });

  if (_intSrv) {
    _server->begin();
    Log.printf("Opened serial web server on http://%s:%d%s\n", WiFi.localIP().toString().c_str(), _webPort, _prefix.c_str());
    MDNS.addService("http", "tcp", _webPort);
  } else 
    Log.printf("Added serial web server on %s\n", _prefix.c_str());
};

void WebSerialStream::stop() {
  if (!_server)
    return;
  if (!_intSrv)
    return;
  _server->end();
  delete _server;
  _server = NULL;
}

void WebSerialStream::loop() {
  _ws->cleanupClients();
}
#endif
