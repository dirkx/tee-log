#include "Log.h"
#include "WebSerialStream.h"
#include <ESPmDNS.h>

size_t WebSerialStream::write(uint8_t c) {
  if (_at>=sizeof(_buff)-1) {
        _at = sizeof(_buff)/2;
	memcpy(_buff, _buff-_at, _at);
  };
  _buff[_at++] = c;
  return 1;
}

WebSerialStream::~WebSerialStream() {
  stop();
}

void WebSerialStream::begin() {
  if (_server != NULL)
    return;
  _server = new WebServer(_webPort);
  _server->on("/",[this]() {
  _server->send(200, "text/html", "<html><head><title>log</title></head>"
	"<style>"
		"#log { font-family: 'Courier New', monospace; white-space: pre; }"
	"</style>"
	"<script language=javascript>"
	"window.onLoad = setInterval(function () { fetch('log')."
                "then("
		   "function(r) { return r.text() }"
                ").then( "
                   "function(txt) { document.getElementById('log').innerHTML += txt; }"
		")}, 750);"
	"</script>"
	"<body><div id=log></div></body></html>");
  });
  _server->on("/log",[this]() {
    _buff[_at] = 0;
    _server->send(200, "text/plain",String((const char*)_buff));
	return json with at and payload; rely on browser to give at; at is size_t and done modulo.

    _at = 0; // just one...
  });
  _server->begin();

  Log.printf("Opened serial web server on http://%s:%d\n", WiFi.localIP().toString().c_str(), _webPort);
  MDNS.addService("http", "tcp", _webPort);
};

void WebSerialStream::stop() {
  if (!_server)
    return;
  _server->stop();
  delete _server;
  _server = NULL;
}

void WebSerialStream::loop() {
  if (_server)
 	_server->handleClient();
}
