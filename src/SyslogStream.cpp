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
#include "SyslogStream.h"

#if (defined(ESP32) || defined(ESP8266))

size_t SyslogStream::write(uint8_t c) {

  if (at >= sizeof(logbuff) - 1) {
    Log.println("Purged logbuffer (should never happen)");
    at = 0;
  };

  if (c >= 32 && c < 128)
    logbuff[ at++ ] = c;

  if (c == '\n' || at >= sizeof(logbuff) - 1) {

    logbuff[at++] = 0;
    at = 0;

    if (_logging) {
      WiFiUDP syslog;

      if (syslog.begin(_syslogPort)) {
        if (_useIpDestination){
          syslog.beginPacket(_destIp, _syslogPort);
        }
        else if (_destHost){
          syslog.beginPacket(_destHost, _syslogPort);
        }
        else{
          syslog.beginPacket(WiFi.gatewayIP(), _syslogPort);
        }
        if (_raw)
          syslog.printf("%s\n", logbuff);
        else {
          time_t now = time(NULL);
          char * p = (char *)"noTime";
          if (now > 3600) {
            //  0123456789012345678 9 0
            // "Thu Nov  4 09:47:43\n\0" -> 09:47\0
            p = ctime(&now);
            p += 4; // See section 4.1.2 of RFC 4164
            p[strlen(p) - 1] = 0;
          };
          syslog.printf("<135> %s %s %s", p, identifier().c_str(), logbuff);
        };
        syslog.endPacket();
      };
    };
  };
  return 1;
}
#endif
