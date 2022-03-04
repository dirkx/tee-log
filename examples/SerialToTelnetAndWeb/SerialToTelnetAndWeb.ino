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

#include <TLog.h>      // The T-Logging library.

// Run a telnet service on the default port (23) which shows what is
// sent to Serial if you telnet to it.
//
#include <TelnetSerialStream.h>
TelnetSerialStream telnetSerialStream = TelnetSerialStream();

// Likewise let http://<ipaddres>:80/ show the log in a webbrowser.
//
#include <WebSerialStream.h>
WebSerialStream webSerialStream = WebSerialStream();

void setup() {
  Serial.begin(115200);
  Log.println("Started (this will only show up on serial - nothing setup and no network)");

  Log.addPrintStream(std::make_shared<TelnetSerialStream>(telnetSerialStream));
  Log.addPrintStream(std::make_shared<WebSerialStream>(webSerialStream));

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWD);
  while (!WiFi.isConnected()) {
    delay(500);
    Log.print(".");
  }
  Log.println("Connected!");

  // Set up mDNS to make our serial-2-telnet and http service visible and easy to find.
  MDNS.begin("my-webby-name");
  
  Log.begin();
}

void loop() {
  // take care of any TLog.housekeeping; such as flushing any buffers
  // with log data.
  Log.loop();

  // Say something every seconds.
  static unsigned  long last_report = millis();
  if (millis() - last_report < 1 * 1000)
    return;

  static int i = 0;
  Log.printf("Hello number %d from the loop\n", i++);
  last_report = millis();
};
