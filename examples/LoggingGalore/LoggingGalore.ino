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

// Uncomment to activate syslog logging
// #define SYSLOG_HOST "loghost.my.domain.com"

// Uncomment to activate mqtt logging
// #define MQTT_HOST "mqttserver.my.domain.com"

// We will always run a telnet server to telnet to and see
// the serial log output.
//
#include <TelnetSerialStream.h>
TelnetSerialStream telnetSerialStream = TelnetSerialStream();

// Likewise let http://<ipaddres>:80/ show the log in a webbrowser.
//
#include <WebSerialStream.h>
WebSerialStream webSerialStream = WebSerialStream();

// Only send it to syslog if we have a host defined.
//
#ifdef SYSLOG_HOST
#include <SyslogStream.h>
SyslogStream syslogStream = SyslogStream();
#endif

// Only send it to MQTT if we have a host defined
//
#ifdef MQTT_HOST
#include <MqttlogStream.h>
// EthernetClient client;
WiFiClient client;
MqttStream mqttStream = MqttStream(&client);
char topic[128] = "log/foo";
#endif

void setup() {
  Serial.begin(115200);
  Serial.println("Started (this will only show up on serial)");

  Log.addPrintStream(std::make_shared<TelnetSerialStream>(telnetSerialStream));
  Log.addPrintStream(std::make_shared<WebSerialStream>(webSerialStream));

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWD);
  while(!WiFi.isConnected()) {
    Log.println("No network yet");
    delay(1000);
  }
  Log.println("We have network");
  

#ifdef SYSLOG_HOST
  syslogStream.setDestination(SYSLOG_HOST);
  syslogStream.setRaw(false); // wether or not the syslog server is a modern(ish) unix.
#ifdef SYSLOG_PORT
  syslogStream.setPort(SYSLOG_PORT);
#endif

  const std::shared_ptr<LOGBase> syslogStreamPtr = std::make_shared<SyslogStream>(syslogStream);
  Log.addPrintStream(syslogStreamPtr);
#endif

#ifdef MQTT_HOST
  mqttStream.setServer(MQTT_HOST);
  mqttStream.setTopic(topic);

  const std::shared_ptr<LOGBase> mqttStreamPtr = std::make_shared<MqttStream>(mqttStream);
  Log.addPrintStream(mqttStreamPtr);
#endif

  Log.begin();
  Log.println("We are done setting up");

  // Call mDNS to make our serial-2-telnet service visible and easy to find.
  MDNS.begin("chatty-server");
}

void loop() {
  Log.loop();

  // do something every 5 seconds.
  static unsigned  long last_report = millis();
  if (millis() - last_report < 5 * 1000)
    return;

  Log.println("Hello from the loop");
  last_report = millis();
};
