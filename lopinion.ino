/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
 
SoftwareSerial GPRS(7, 8);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0x6F, 0xC0 };
IPAddress ip(192,168,0, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

const int numberOfAnswers = 5;
char* answers[] = {"Banana", "Orange", "Strawberry", "Raspberry", "Apple"};
int votes[numberOfAnswers] = {0, 0, 0, 0, 0};
int totalVotes = 0;

String buffer = "";                // buffer to store bytes coming from GPRS shield
String smsContent = "";            // SMS content (i.e. a digit)
String smsIndex = "";              // SMS index (i.e. a number)
boolean readingSmsIndex;           // if you're currently reading the SMS index
boolean readingSmsContent = false; // if you're currently reading the SMS content


// temp vars
unsigned char sms[64]; // buffer array for data recieve over serial port
int count=0;     // counter for buffer array 

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("PollServer v0.1 starting...");
  
  GPRS.begin(19200);  // the GPRS baud rate
  
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Serial.println("failed to get an IP address using DHCP, trying manually");
    Ethernet.begin(mac, ip);
  }
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  // get any incoming bytes from GPRS:
  if (GPRS.available()) {
    while(GPRS.available()) {
      char inChar = GPRS.read();
      Serial.write(inChar);
      
      // if we get a <CR>, process the Buffer:
      if (inChar == 13) {
        processBuffer();
        buffer = "";
      }
      
      // if we get a <CR>, process the Buffer:
      if (inChar == 10) {
        processBuffer();
      }
      else {
        buffer += String(inChar);
      }
      
      char inChar = GPRS.read();
      buffer += inChar;
      
      // if you get a newline, clear the line:
      if (inChar == '\n') {
        buffer = "";
      }
      
      Serial.write(inChar);
      
      // if a PIN code is needed
      if ( buffer.endsWith("+CPIN")) {
        // Send it
        GPRS.println("AT+CPIN=\"0000\"");
        buffer = "";
        break;
      }
      
      // if we received an SMS
      if (buffer.endsWith("+CMTI")) {
        readingSmsIndex = true;
        smsIndex = "";
      }
      
      // if you're currently reading the SMS index
      // add them to the smsIndex String:
      if (readingSmsIndex) {
        // we just want the char after ','
        if (inChar == ',') {
          smsIndex = "";
        } 
        if (inChar != '\n') {
          smsIndex += inChar;
        } 
        else {
          // we've reached the end of the line:
          readingSmsIndex = false;
          GPRS.print("AT+CMGR=");
          for (int j = 1; j < smsIndex.length(); j++) {
            GPRS.print(smsIndex.charAt(j));
          }
        }
      }
        
      // if we sent AT+CMGR command
      if (buffer.endsWith("READ")) {
        readingSmsContent = true;
        smsContent = "";
      }
      
      // if you're currently reading the SMS index
      // add them to the smsIndex String:
      if (readingSmsContent) {
         while(GPRS.available()) {
           char inChar2 = GPRS.read();
           Serial.write(inChar2);
         }
      }
      
      if(buffer.length() >= 64)
        break;
    }
    buffer = "";
  }
  
  // get any incoming bytes from Serial:
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    byte digit;
    
    if(isDigit(inByte)) {
      switch (inByte) {
        case '0':
          digit = 0;
          break;
        case '1':
          digit = 1;
          break;
        case '2':
          digit = 2;
          break;
        case '3':
          digit = 3;
          break;
        case '4':
          digit = 4;
          break;
        case '5':
          digit = 5;
          break;
        case '6':
          digit = 6;
          break;
        case '7':
          digit = 7;
          break;
        case '8':
          digit = 8;
          break;
        case '9':
          digit = 9;
          break;
      }
      if(digit >= 0 && digit < numberOfAnswers) {
        totalVotes ++;
        votes[digit] ++;
        Serial.print(votes[digit]);
        Serial.print(" votes for  \"");
        Serial.print(answers[digit]);
        Serial.print("\" (");
        Serial.print(totalVotes);
        Serial.println(" votes total)");
      }
    }
  }
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("	<head>"));
          client.println(F("		<meta charset=\"utf-8\">"));
          client.println(F("		<meta http-equiv=\"refresh\" content=\"30\">"));
          client.println(F("		<style>"));
          client.println(F("			body {"));
          client.println(F("			  font: 10px sans-serif;"));
          client.println(F("			}"));
          client.println(F("			.axis path,"));
          client.println(F("			.axis line {"));
          client.println(F("			  fill: none;"));
          client.println(F("			  stroke: #000;"));
          client.println(F("			  shape-rendering: crispEdges;"));
          client.println(F("			}"));
          client.println(F("			.bar {"));
          client.println(F("			  fill: steelblue;"));
          client.println(F("			}"));
          client.println(F("			.x.axis path {"));
          client.println(F("			  display: none;"));
          client.println(F("			}"));
          client.println(F("		</style>"));
          client.println(F("	</head>"));
          client.println(F("	<body>"));
          client.println(F("		<script src=\"http://d3js.org/d3.v3.min.js\"></script>"));
          client.println(F("		<script>"));
          client.println(F("			var margin = {top: 20, right: 20, bottom: 30, left: 40},"));
          client.println(F("				width = 1300 - margin.left - margin.right,"));
          client.println(F("				height = 720 - margin.top - margin.bottom;"));
          client.println(F("			var formatPercent = d3.format(\".0%\");"));
          client.println(F("			"));
          client.println(F("			var color = d3.scale.ordinal()"));
          client.println(F("				.range([\"#fce94f\", \"#8ae234\", \"#fcaf3e\", \"#729fcf\", \"#ad7fa8\", \"#ef2929\", \"#babdb6\"]);"));
          client.println(F("			var x = d3.scale.ordinal()"));
          client.println(F("				.rangeRoundBands([0, width], .1);"));
          client.println(F("			var y = d3.scale.linear()"));
          client.println(F("				.range([height, 0]);"));
          client.println(F("			var xAxis = d3.svg.axis()"));
          client.println(F("				.scale(x)"));
          client.println(F("				.orient(\"bottom\");"));
          client.println(F("			var yAxis = d3.svg.axis()"));
          client.println(F("				.scale(y)"));
          client.println(F("				.orient(\"left\")"));
          client.println(F("				.tickFormat(formatPercent);"));
          client.println(F("			var svg = d3.select(\"body\").append(\"svg\")"));
          client.println(F("				.attr(\"width\", width + margin.left + margin.right)"));
          client.println(F("				.attr(\"height\", height + margin.top + margin.bottom)"));
          client.println(F("			  .append(\"g\")"));
          client.println(F("				.attr(\"transform\", \"translate(\" + margin.left + \",\" + margin.top + \")\");"));
          client.print(F("			var data = ["));
          for (int i = 0; i < numberOfAnswers; i++) {
            client.print("{\"answer\": \"");
            client.print(answers[i]);
            client.print("\", \"percentage\": \"");
            client.print(float(votes[i])/float(totalVotes));
            //client.print(0.10);
            client.print("\"}");
            if(i != (numberOfAnswers-1))
              client.print(",");
          }
          client.println(F("];"));
          client.println(F("			x.domain(data.map(function(d) { return d.answer; }));"));
          client.println(F("			y.domain([0, d3.max(data, function(d) { return d.percentage; })]);"));
          client.println(F("			svg.append(\"g\")"));
          client.println(F("			  .attr(\"class\", \"x axis\")"));
          client.println(F("			  .attr(\"transform\", \"translate(0,\" + height + \")\")"));
          client.println(F("			  .call(xAxis);"));
          client.println(F("			  "));
          client.println(F("			svg.append(\"g\")"));
          client.println(F("			  .attr(\"class\", \"y axis\")"));
          client.println(F("			  .call(yAxis)"));
          client.println(F("			.append(\"text\")"));
          client.println(F("			  .attr(\"transform\", \"rotate(-90)\")"));
          client.println(F("			  .attr(\"y\", 6)"));
          client.println(F("			  .attr(\"dy\", \".71em\")"));
          client.println(F("			  .style(\"text-anchor\", \"end\")"));
          client.println(F("			  .text(\"Pourcentage des votes\");"));
          client.println(F("			svg.selectAll(\".bar\")"));
          client.println(F("			  .data(data)"));
          client.println(F("			.enter().append(\"rect\")"));
          client.println(F("			  .attr(\"class\", \"bar\")"));
          client.println(F("			  .attr(\"x\", function(d) { return x(d.answer); })"));
          client.println(F("			  .attr(\"width\", x.rangeBand())"));
          client.println(F("			  .style(\"fill\", function(d, i) { return color(i); })"));
          client.println(F("			  .attr(\"y\", function(d) { return y(d.percentage); })"));
          client.println(F("			  .attr(\"height\", function(d) { return height - y(d.percentage); });"));
          client.println(F("		</script>"));
          client.println(F("	</body>"));
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

