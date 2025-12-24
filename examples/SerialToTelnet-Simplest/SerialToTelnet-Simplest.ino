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

#include <TLog.h>  // The T-Logging library.

// Run a telnet service on the default port (23) which shows what is
// sent to Serial if you telnet to it.
//
#include <TelnetSerialStream.h>

// TelnetSerialStream telnetSerialStream = TelnetSerialStream("myhostname");

// Other options are to run it on a non standard port; e.g. 666, meaning
// you have to telnet to port 666 rather than the default 23.
//
// TelnetSerialStream telnetSerialStream = TelnetSerialStream(666);
//
// Or to allow more than the default MAX_SERIAL_TELNET_CLIENTS (4 by default)
// simultaneous connections (a 100 in this case).
//
// TelnetSerialStream telnetSerialStream = TelnetSerialStream(666,100);

void setup() {
  Serial.begin(115200);
  Serial.println("Started (this will only show up on serial)");

  // This will also show up on just the serial - as we have no telnet
  // wired up yet; nor is there a network.
  //
  Log.println("Logging before the network is up (and we only have serial wired up at this point)");

  Log.addPrintStream(std::make_shared<TelnetSerialStream>("host.domain.com"));

  // This will also show up on just the serial - no network yet.
  //
  Log.println("Logging before the network is up - but after adding telnet");

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWD);
  int i = 1;
  while (!WiFi.isConnected()) {
    Log.printf("No network yet (%d seconds)\n", i++);
    delay(1000);
  }
  // Call mDNS to make our serial-2-telnet service visible and easy to find.
  MDNS.begin("my-name");

  // Sometimes this is really nice to confirm that you are connecting to
  // the hardware you think you are.
  Log.setIdentifier(WiFi.macAddress().c_str());

  Log.begin();
  Log.print("We have network. You can telnet to ");
  Log.print(WiFi.localIP());
  Log.println(" to see the logging output (including the past if you are fast enough)");
}

static int counter = 0;
void loop() {
  // take care of any TLog.housekeeping; such as flushing any buffers
  // with log data.
  Log.loop();

  // Say something every 5 seconds.
  static unsigned long last_report = millis();
  if (millis() - last_report < 2 * 1000)
    return;

  Log.printf("Hello from the loop - the count is %d\n", counter++);
  last_report = millis();
  if (rand() & 1)
    Log.printf("Something extra %d\n", rand());

  Log.disableSerial(true);
  Log.println("This log entry is not visible in the serial console");
  Log.disableSerial(false);
};
