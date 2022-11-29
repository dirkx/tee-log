/* Copyright 2008, 2012-2022 Dirk-Willem van Gulik <dirkx(at)webweaving(dot)org>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Library that provides a fanout, or T-flow; so that output or logs do
   not just got to the serial port; but also to a configurable mix of a
   telnetserver, a webserver, syslog or MQTT.
*/

#include <TLog.h>      // The T-Logging library.

//
#include <LogStream.h>
LogStream serial1Log;
LogStream serial2Log;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial2.begin(4800);

  Serial.println("Started (this will only show up on serial)");

  Log.addPrintStream(std::make_shared<LogStream>(serial1Log));
  Log.addPrintStream(std::make_shared<LogStream>(serial2Log));

  serial1Log.begin(Serial1);
  serial2Log.begin(Serial2);

  Log.begin();
  Log.println("All channels");
  serial2Log.println("Just 2");
}

void loop() {
  // take care of any TLog.housekeeping; such as flushing any buffers
  // with log data.
  Log.loop();

  // Say something every 5 seconds.
  static unsigned  long last_report = millis();
  if (millis() - last_report < 5 * 1000)
    return;

  static int i = 0;
  if (i & 4 )
    Log.println("All channels (Serial, Serial1, Serial2");
  if (i & 1 )
    serial1Log.println("Just Serial 1");
  if (i & 2 )
    serial2Log.println("Just Serial 12");
};