Tee Logger

A logger that acts as a fan-out or tee; i.e. lets you sent the logs to the Serial port, but also
to (for example) SYSLOG, a local server you can 'telnet' into or make it scroll in a browser window.

Example:

	#include <Log.h>
	TelnetSerialStream telnetSerialStream = TelnetSerialStream();

	..
	void setup() {
		Serial.begin(115200);
		Serial.println("Started (this will only show up on serial)");

		... start wifi network ...

                Serial.print("From this point onward; telnet to ");
                Serial.print(WiFi.localIP());
                Serial.println(" to see the logging.");

  		Log.addPrintStream(std::make_shared<TelnetSerialStream>(telnetSerialStream));
		Log.begin();

		Log.println("Hello World");


        void loop() {
                 // take care of any TLog.housekeeping; such as flushing any buffers
                 // with log data.
                 Log.loop();
  

With this setup; the output "Hello World" is visible both on the Serial port; as well as on 
a local telnet server. So doing a telnet to the ESP32; will show this:

	$ telnet 10.0.0.2
  	Connected to the Serial port of 10.0.0.2
	Hello World

where 10.0.0.2 is the IP address of the ESP32 board. Likewise you can add a logger that
sends the logging output to a syslog server; to MQTT or to a webserver. In that case
you can simply open a webbrowser and connect to http://10.0.0.2 (i.e. to the IP address
of the EPS32) and see the output scroll.

Note that you can still stream to the indivudal loggers; e.g. in above example

        telnetSerialStream.println("Telnet only");

works too. Or see the Serial1/Serial2 example.

It is used at the https://makerspaceleiden.nl/ for its door access: https://github.com/makerspaceleiden.
