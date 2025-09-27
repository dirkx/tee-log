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
#ifndef WIFI_NETWORK
#warning "You really want to change this !"
#define WIFI_NETWORK "MyWiFiNetwork"
#define WIFI_PASSWD "MySecretPassword"
#endif

#include <TLog.h>
#include <MqttlogStream.h>

#ifndef MQTT_HOST
#warning "You really want to change this !"
// See https://www.hivemq.com/mqtt/public-mqtt-broker/ for details.
#define MQTT_HOST "broker.hivemq.com"
#endif

// We will always run a telnet server to telnet to and see
// the serial log output.
//
#include <TelnetSerialStream.h>
TelnetSerialStream telnetSerialStream = TelnetSerialStream();

// EthernetClient client;
WiFiClient client;
MqttStream mqttStream = MqttStream(&client);
char topic[128] = "log/foo";

void setup() {
  Serial.begin(115200);
  Serial.println("Started (this will only show up on serial)");

  Log.addPrintStream(std::make_shared<TelnetSerialStream>(telnetSerialStream));

  mqttStream.setServer(MQTT_HOST);
  mqttStream.setTopic(topic);
  Log.addPrintStream(std::make_shared<MqttStream>(mqttStream));

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWD);
  while(!WiFi.isConnected()) {
    Log.println("No network yet");
    delay(1000);
  }
  Log.println("We have network");

  Log.begin();
  Log.println("We are done setting up");

  // Call mDNS to make our serial-2-telnet service visible and easy to find.
  MDNS.begin("chatty-server");

  Log.println("Once started - go to https://www.hivemq.com/demos/websocket-client/ to");
  Log.print("see the MQTT data. The hostname to fill out is ");
  Log.print(MQTT_HOST);
  Log.print(" with topic ");
  Log.print(topic);
  Log.println(".");
}

int cnt = 0;
void loop() {
  Log.loop();

  // do something every 5 seconds.
  static unsigned  long last_report = millis();
  if (millis() - last_report < 5 * 1000)
    return;

  Log.printf("Hello from the loop nummer %06d\n",++cnt);
  last_report = millis();
};
