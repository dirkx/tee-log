// Simple 'tee' class - that sends all 'serial' port data also to the Syslog and/or MQTT bus -
// to the 'log' topic if such is possible/enabled.
//
// XXX should refactor in a generic buffered 'add a Stream' class and then
// make the various destinations classes in their own right you can 'add' to the T.
//
//
#include "Log.h"
#include "SyslogStream.h"
#include "WiFi.h"

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

    {
      WiFiUDP syslog;

      if (syslog.begin(_syslogPort)) {
        if (_dest)
          syslog.beginPacket(_dest, _syslogPort);
        else
          syslog.beginPacket(WiFi.gatewayIP(), _syslogPort);
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
          syslog.printf("<135> %s %s %s", p, identifier(), logbuff);
        };
        syslog.endPacket();
      };
    };
  };
  return 1;
}
