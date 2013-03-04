/*
  L'opinion
 This is a simple poll server powered by D3.js (a js library), an  Arduino Ethernet and a GPRS shield.
  
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * GPRS shield attached to pins 7, 8
 
 modified 4 Mar 2013
 by Baptiste Gaultier
 
 This code is in the public domain.
 
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

const int numberOfLabels = 6;
char* labels[] = {"Goyave", "Banane", "Cerise", "Ananas", "Papaye", "Carambole"};
char* colors[] = {"#859d94", "#7cc1cb", "#b1cd67", "#d17a5b", "#95aaa2", "#90d1da", "#bacf83"};
int votes[numberOfLabels] = {0, 0, 0, 0, 0, 0};
int totalVotes = 0;


String currentLine = "";           // buffer to store bytes coming from GPRS shield
String smsIndex = "";              // SMS index (i.e. a number)
String smsContent = "";            // SMS content (i.e. a digit)
boolean readingSmsIndex = false;    // if we're currently reading SMS index
boolean readingSmsStore = false;    // if we're currently reading SMS store
boolean readingSmsContent = false;  // if we're currently reading SMS content


void setup() {
  // reserve space for the strings:
  currentLine.reserve(256);
  
  // Open serial communications :
  Serial.begin(19200);
  Serial.println("PollServer v0.3 starting...");
  
  // GPRS shield baud rate
  GPRS.begin(19200);
  
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
      // read incoming bytes:
      char inChar = GPRS.read();
      Serial.write(inChar);

      // add incoming byte to end of line:
      currentLine += inChar;
      
      if ( currentLine.endsWith("+CPIN: SIM PIN")) {
        // send our PIN code
        GPRS.println("AT+CPIN=\"0000\"");
        currentLine = "";
      }
      
      // if the current line ends with +CMTI, it will
      // be followed by SMS index
      if ( currentLine.endsWith("+CMTI")) {
        // smsIndex is beginning. Clear the SmsIndex string:
        readingSmsIndex = true; 
        smsIndex = "";
      }
      
      // if you're currently reading the bytes of a SmsIndex,
      // add them to the SmsIndex String:
      if (readingSmsIndex) {
        if (inChar != '\n')
         smsIndex += inChar;
        else {
         // if you got a "\n" character,
         // you've reached the end of the line:
         readingSmsIndex = false;
         // currentLine is just used as a buffer here
          currentLine = smsIndex.substring(smsIndex.indexOf( "," )+1);
         //Serial.println(currentLine);
         // search for SMS with the followinf index in SMS store
         GPRS.print("AT+CMGR=");
         GPRS.print(currentLine);
         delay(1000);
         GPRS.print("AT+CMGR=");
         GPRS.print(currentLine);
          currentLine = "";
        }
      }
      
      // if the current line ends with +UNREAD, it will
      // be followed by SMS store line content
      if ( currentLine.endsWith("REC READ")) {
        readingSmsContent = true;
        smsContent = "";
      }
      
      if (readingSmsContent)
        smsContent += inChar;
      }
    }
    if (readingSmsContent) {
      /*Serial.print("SMS content : ");
      Serial.print(smsContent.charAt(smsContent.length() - 2));
      Serial.println(".");*/
      addVote(smsContent.charAt(smsContent.length() - 2));
      readingSmsContent = false;
    }
  
  // get any incoming bytes from Serial :
  if (Serial.available()) {
    int inByte = Serial.read();
    GPRS.write(inByte); // send them to the GPRS shield
    if(isDigit(inByte))
      addVote(inByte);
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
          client.println("<!DOCTYPE HTML>");
          client.println(F("<html>"));
          client.println(F("<meta charset=\"utf-8\">"));
          client.println(F("<style>"));
          client.println(F("body {"));
          client.println(F("  font-family: \"Ubuntu\", Helvetica, Arial, sans-serif;"));
          client.println(F("  width: 100%;"));
          client.println(F("  background-color: #e8f0ed;"));
          client.println(F("  color: #40564e;"));
          client.println(F("}"));
          client.println(F("text {"));
          client.println(F("  fill :  #40564e;"));
          client.println(F("}"));
          client.println(F("#bar {"));
          client.println(F("	font-size : 10px;"));
          client.println(F("}"));
          client.println(F(".axis path,"));
          client.println(F(".axis line {"));
          client.println(F("  fill: none;"));
          client.println(F("  stroke: #40564e;"));
          client.println(F("  shape-rendering: crispEdges;"));
          client.println(F("}"));
          client.println(F(".bar, .box, .arc {"));
          client.println(F("  fill-opacity: .9;"));
          client.println(F("}"));
          client.println(F(".x.axis path {"));
          client.println(F("  display: none;"));
          client.println(F("}"));
          client.println(F(".details {"));
          client.println(F("  color: #777777;"));
          client.println(F("  font-size: 10px;"));
          client.println(F("  line-height: 14px;"));
          client.println(F("}"));
          client.println(F("</style>"));
          client.println(F("<body>"));
          client.println(F("<h1 style=\"margin-bottom : 10px;\">Quel est votre fruit préféré ?</h1>"));
          client.println(F("<div class=\"details\">"));
          client.println(F("  <span>Sondage lancé par Baptiste</span>"));
          client.println(F("  <span> | </span>"));
          client.println(F("  <span>2 participants</span>"));
          client.println(F("  <span> | </span>"));
          client.println(F("  <span>il y a moins d'une minute</span>"));
          client.println(F("  <label><input type=\"checkbox\"> Trier les résultats</label>"));
          client.println(F("</div>"));
          client.println(F("<script src=\"http://d3js.org/d3.v3.min.js\"></script>"));
          client.println(F("<script>"));
          client.println(F("// bar chart"));
          client.println(F("var margin = {top: 20, right: 20, bottom: 30, left: 40},"));
          client.println(F("    width = 480 - margin.left - margin.right,"));
          client.println(F("    height = 300 - margin.top - margin.bottom;"));
          client.println(F("var formatPercent = d3.format(\".0%\");"));
          client.println(F("var x = d3.scale.ordinal()"));
          client.println(F("    .rangeRoundBands([0, width], .1, 1);"));
          client.println(F("var y = d3.scale.linear()"));
          client.println(F("    .range([height, 0]);"));
          client.println(F("var xAxis = d3.svg.axis()"));
          client.println(F("    .scale(x)"));
          client.println(F("    .orient(\"bottom\");"));
          client.println(F("var yAxis = d3.svg.axis()"));
          client.println(F("    .scale(y)"));
          client.println(F("    .orient(\"left\")"));
          client.println(F("    .tickFormat(formatPercent);  "));
          client.println(F("var data = ["));
          for (int i = 0; i < numberOfLabels; i++) {
            client.print("{\"label\": \"");
            client.print(labels[i]);
            client.print("\", \"percentage\": \"");
            client.print(float(votes[i])/float(totalVotes));
            client.print("\", \"color\": \"");
            client.print(colors[i]);
            client.print("\"}");
           if(i != (numberOfLabels-1))
              client.print(",");
          }
          client.println(F("];"));
          client.println(F("var svg = d3.select(\"body\").append(\"svg\")"));
          client.println(F("		.attr(\"id\", \"bar\")"));
          client.println(F("    .attr(\"width\", width + margin.left + margin.right)"));
          client.println(F("    .attr(\"height\", height + margin.top + margin.bottom)"));
          client.println(F("  .append(\"g\")"));
          client.println(F("  .attr(\"transform\", \"translate(\" + margin.left + \",\" + margin.top + \")\");"));
          client.println(F("data.forEach(function(d) {"));
          client.println(F("  d.percentage = +d.percentage;"));
          client.println(F("});"));
          client.println(F("x.domain(data.map(function(d) { return d.label; }));"));
          client.println(F("y.domain([0, d3.max(data, function(d) { return d.percentage; })]);"));
          client.println(F("svg.append(\"g\")"));
          client.println(F("    .attr(\"class\", \"x axis\")"));
          client.println(F("    .attr(\"transform\", \"translate(0,\" + height + \")\")"));
          client.println(F("    .call(xAxis);"));
          client.println(F("svg.append(\"g\")"));
          client.println(F("    .attr(\"class\", \"y axis\")"));
          client.println(F("    .call(yAxis)"));
          client.println(F("  .append(\"text\")"));
          client.println(F("    .attr(\"transform\", \"rotate(-90)\")"));
          client.println(F("    .attr(\"y\", 6)"));
          client.println(F("    .attr(\"dy\", \".71em\")"));
          client.println(F("    .style(\"text-anchor\", \"end\")"));
          client.println(F("    .text(\"pourcentage\");"));
          client.println(F("svg.selectAll(\".bar\")"));
          client.println(F("    .data(data)"));
          client.println(F("  .enter().append(\"rect\")"));
          client.println(F("    .attr(\"class\", \"bar\")"));
          client.println(F("    .attr(\"x\", function(d) { return x(d.label); })"));
          client.println(F("    .attr(\"width\", x.rangeBand())"));
          client.println(F("    .attr(\"y\", function(d) { return y(d.percentage); })"));
          client.println(F("    .style(\"fill\", function(d) { return d.color; })"));
          client.println(F("    .attr(\"height\", function(d) { return height - y(d.percentage); });"));
          client.println(F("d3.select(\"input\").on(\"change\", change);"));
          client.println(F("var sortTimeout = setTimeout(function() {"));
          client.println(F("  d3.select(\"input\").property(\"checked\", true).each(change);"));
          client.println(F("}, 2000);"));
          client.println(F("function change() {"));
          client.println(F("    clearTimeout(sortTimeout);"));
          client.println(F("    // Copy-on-write since tweens are evaluated after a delay."));
          client.println(F("    var x0 = x.domain(data.sort(this.checked"));
          client.println(F("        ? function(a, b) { return b.percentage - a.percentage; }"));
          client.println(F("        : function(a, b) { return d3.ascending(a.label, b.label); })"));
          client.println(F("        .map(function(d) { return d.label; }))"));
          client.println(F("        .copy();"));
          client.println(F("    var transition = svg.transition().duration(750),"));
          client.println(F("        delay = function(d, i) { return i * 50; };"));
          client.println(F("    transition.selectAll(\".bar\")"));
          client.println(F("        .delay(delay)"));
          client.println(F("        .attr(\"x\", function(d) { return x0(d.label); });"));
          client.println(F("    transition.select(\".x.axis\")"));
          client.println(F("        .call(xAxis)"));
          client.println(F("      .selectAll(\"g\")"));
          client.println(F("        .delay(delay);"));
          client.println(F("  }"));
          client.println(F("// pie chart"));
          client.println(F("var width = 300,"));
          client.println(F("    height = 300,"));
          client.println(F("    radius = Math.min(width, height) / 2;"));
          client.println(F("var pie = d3.layout.pie()"));
          client.println(F("    .sort(function(a, b) { return b.percentage - a.percentage; })"));
          client.println(F("    .value(function(d) { return d.percentage; });"));
          client.println(F("var arc = d3.svg.arc()"));
          client.println(F("    .innerRadius(radius - 60)"));
          client.println(F("    .outerRadius(radius - 20);"));
          client.println(F("var piesvg = d3.select(\"body\").append(\"svg\")"));
          client.println(F("    .attr(\"width\", width)"));
          client.println(F("    .attr(\"height\", height)"));
          client.println(F("  .append(\"g\")"));
          client.println(F("    .attr(\"transform\", \"translate(\" + width / 2 + \",\" + height / 2 + \")\");"));
          client.println(F("var path = piesvg.selectAll(\"path\")"));
          client.println(F("    .data(pie(data))"));
          client.println(F("  .enter().append(\"path\")"));
          client.println(F("    .style(\"fill\", function(d) { return d.data.color; })"));
          client.println(F("    .attr(\"class\", \"arc\")"));
          client.println(F("    .attr(\"d\", arc);"));
          client.println(F("var text = piesvg.selectAll(\"text\")"));
          client.println(F("    .data(pie(data))"));
          client.println(F("  .enter().append(\"text\")"));
          client.println(F("      .attr(\"transform\", function(d) { return \"translate(\" + arc.centroid(d) + \")\"; })"));
          client.println(F("      .attr(\"dy\", \".35em\")"));
          client.println(F("      .style(\"text-anchor\", \"middle\")"));
          client.println(F("      .style(\"font\", \"10px sans-serif\")"));
          client.println(F("      .text(function(d) { return d.data.label; }); // store the initial values"));
          client.println(F("   "));
          client.println(F("var boxsvg = d3.select(\"body\").append(\"svg\")"));
          client.println(F("							 .attr(\"width\", data.length * 120)"));
          client.println(F("							 .style(\"margin\", \"40px\");"));
          client.println(F("var g = boxsvg.selectAll(\".box\")"));
          client.println(F("      .data(data)"));
          client.println(F("    .enter().append(\"g\")"));
          client.println(F("      .attr(\"class\", \"box\")"));
          client.println(F("      .attr(\"transform\", function(d, i) { return \"translate(\" + i * 120 + \", 0 )\"; });"));
          client.println(F("      "));
          client.println(F("g.append(\"rect\")"));
          client.println(F("      .attr(\"width\", 100)"));
          client.println(F("      .attr(\"height\", 100)"));
          client.println(F("      .style(\"fill\", function(d) { return d.color; });"));
          client.println(F("g.append(\"text\")"));
          client.println(F("			.attr(\"dx\", \"18px\")"));
          client.println(F("      .attr(\"dy\", \"76px\")"));
          client.println(F("      .attr(\"font-size\", \"10px\")"));
          client.println(F("      .text(function(d, i) { return \"Envoyez \" + i + \" au\"; });   "));
          client.println(F("g.append(\"text\")"));
          client.println(F("			.attr(\"dx\", \"14px\")"));
          client.println(F("      .attr(\"dy\", \"90px\")"));
          client.println(F("      .attr(\"font-size\", \"10px\")"));
          client.println(F("      .text(\"06 69 98 71 37\");"));
          client.println(F("      "));
          client.println(F("g.append(\"text\")"));
          client.println(F("			.attr(\"dx\", \"23px\")"));
          client.println(F("      .attr(\"dy\", \"40px\")"));
          client.println(F("      .attr(\"font-size\", \"14px\")"));
          client.println(F("      .text(function(d) { return d.label; });    "));
          client.println(F("</script>"));
          client.println(F("</body>"));
          client.println(F("</html>"));
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

void addVote(int inByte) {
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
  }
  if(digit >= 0 && digit < numberOfLabels) {
    totalVotes ++;
    votes[digit] ++;
    Serial.print(votes[digit]);
    Serial.print(" votes for  \"");
    Serial.print(labels[digit]);
    Serial.print("\" (");
    Serial.print(totalVotes);
    Serial.println(" votes total)");
  }
}
