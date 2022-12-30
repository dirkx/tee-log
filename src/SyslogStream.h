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

#ifndef _H_SyslogStream
#define _H_SyslogStream

#include <Print.h>
#include <TLog.h>

class SyslogStream : public TLog {
  public:
    SyslogStream(const uint16_t syslogPort = 514) : _syslogPort(syslogPort) {};
    void setPort(uint16_t port) { _syslogPort = port; }
    void setDestination(const char * dest) {
      _destHost = dest;
      _useIpDestination = false;
    }
    void setDestination(IPAddress dest) {
      _destIp = dest;
      _useIpDestination = true;
    }
    void setRaw(bool raw) { _raw = raw; }
    virtual size_t write(uint8_t c);
    virtual void begin() { _logging = true; }
    virtual void end() { _logging = false; }
  private:
    const char * _destHost;
    IPAddress _destIp;
    bool _useIpDestination = false;
    uint16_t _syslogPort;
    char logbuff[512]; // 1024 seems to be to large for some syslogd's.
    size_t at = 0;
    bool _raw;
    bool _logging = false;
  protected:
};
#endif
