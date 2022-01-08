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

#include "Log.h"

#include <PubSubClient.h>
#include "MqttlogStream.h"

void MqttStream::begin() {
  if (!_mqttServer || !_mqttTopic || !_mqttPort) {
    Log.printf("Missing%s for MQTT Logging\n",
               _mqttServer ? "" : " server", _mqttTopic ? "" : " topic", _mqttPort ? "" : " port");
    return;
  }
  
  PubSubClient * psc = new PubSubClient(*_client);
  psc->setServer(_mqttServer, _mqttPort);
  psc->connect(_mqttTopic);
  psc->setBufferSize(sizeof(logbuff)+9+strlen(_mqttTopic));
  _mqtt = psc;
  loop();
}

void MqttStream::loop() {
  static unsigned long lst = 0;
  if (!_mqtt)
    return;

  _mqtt->loop();
  if (_mqtt->connected()) 
    return;

  if (lst && millis() - lst < 15000)
    return;

  Log.printf("MQTT connection state: %d (not connected)\n", _mqtt->state());

  if (_mqtt->connect(_mqttTopic)) {
    Log.println("(re)connecting to MQTT");
    lst = 0;
    return;
  };

  Log.println("MQTT (re)connection failed. Will retry");
  lst = millis();
}

size_t MqttStream::write(uint8_t c) {
  if (at >= sizeof(logbuff) - 1) {
    Log.println("Purged logbuffer (should never happen)");
    at = 0;
  };

  if (c >= 32 && c < 128)
    logbuff[ at++ ] = c;

  if (c == '\n' || at >= sizeof(logbuff) - 1) {
    logbuff[at++] = 0;
    at = 0;

    // perhaps we should buffer this - and do this in the main loop().
    if (_mqtt) {
      if (_mqttTopic == NULL)
        _mqtt->publish("debug", logbuff);
      else
        _mqtt->publish(_mqttTopic, logbuff);
    };
    yield();

  };
  return 1;
}
