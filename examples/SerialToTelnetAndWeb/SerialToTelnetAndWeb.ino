#include <WiFi.h>       // asume a wifi internet connection
#include <ESPmDNS.h>    // advertize our service on Zeroconf/mDNS/Bonjour
#include <Log.h>      // The T-Logging library.

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
  // take care of any Log housekeeping; such as flushing any buffers
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
