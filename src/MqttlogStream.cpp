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
#include "MqttlogStream.h"

#if (defined(ESP32) || defined(ESP8266))
#include <PubSubClient.h>

void MqttStream::begin() {
    if (!_mqtt) {
        if (!_mqttServer || !_mqttTopic || !_mqttPort) {
            Log.printf("Missing%s%s%s for MQTT Logging\n",
                       _mqttServer ? "" : " server", _mqttTopic ? "" : " topic", _mqttPort ? "" : " port" );
            return;
        }
        
        PubSubClient * psc = new PubSubClient(*_client);
        psc->setServer(_mqttServer, _mqttPort);
//        psc->setBufferSize(sizeof(logbuff)+9+strlen(_mqttTopic));
        
        Log.printf("Opened log on mqtt:://%s:%d/%s\n", _mqttServer, _mqttPort, _mqttTopic);
        _mqtt = psc;
        _mqtt->connect(_identifier);
    } else {
        if (!_mqttTopic) {
            Log.printf("Missing topic for MQTT Logging\n");
            return;
        };
        Log.printf("Opened mqtt log on topic %s\n", _mqttTopic);
    };
    reconnect();
    loop();
}

void MqttStream::reconnect() {
    if (_mqtt->connect(_mqttTopic)) {
        Log.println("Log:: (re)connected to MQTT");
        return;
    };
    Log.println("Log:: MQTT (re)connection failed. Will retry");
}

void MqttStream::loop() {
    static unsigned long lst = 0;
    if (!_mqtt)
        return;
    
    _mqtt->loop();
    if (_mqtt->connected()) {
        auto it = unsent.begin();
	int i = 0;
        while (it != unsent.end() && i++ < MAX_MQTT_SENT) {
	    String s = String(*it);
            unsent.erase(it++);
	    // Remove the final LF - as MQTT is line oriented on message
	    // level; and will usually show each message as a line.
	    if (s.endsWith("\n")) s.remove(s.length()-1,1);
            _mqtt->publish(_mqttTopic ? _mqttTopic : "debug", s.c_str()); // it->c_str());
        };
        return;
    };

    // When we do not have the client handle - we're sharing a connection
    // with something else. So no need to try to (re)connect, etc.
    if (!_client)
        return;
    
    if (lst && millis() - lst < 15000)
        return;
    
    reconnect();
    lst = millis();
}

void MqttStream::emitLastLine(String s) {
    if (unsent.size() < MAX_MQTT_QUEUE)
    	unsent.push_back(s);
}

size_t MqttStream::write(uint8_t c) {
    // Ignore partial writes; wait for a full line before sending it.
    return 1;
}
#endif
