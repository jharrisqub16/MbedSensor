/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-bme680-sensor-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Wire.h>




#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//#include <Adafruit_SSD1306.h>



#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <WiFi.h>
#include "ESPAsyncWebServer.h"



//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 32 // OLED display height, in pixels




#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 27, 4, 26);


// Replace with your network credentials
const char* ssid = "BT-NPA8MC";
const char* password = "6GkF7rNJQCc7bn";

//Uncomment if using SPI
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 15


//Adafruit_BME680 bme(BME_CS); // hardware SPI
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

float temperature;
float humidity;
float pressure;
float gasResistance;

AsyncWebServer server(80);
AsyncEventSource events("/events");

unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;  // send readings timer

void getBME680Readings(){
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  temperature = bme.temperature;
  pressure = bme.pressure / 100.0;
  humidity = bme.humidity;
  gasResistance = bme.gas_resistance / 1000.0;
}

String processor(const String& var){
  getBME680Readings();
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "HUMIDITY"){
    return String(humidity);
  }
  else if(var == "PRESSURE"){
    return String(pressure);
  }
 else if(var == "GAS"){
    return String(gasResistance);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>BME680 Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: grey; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: black; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .card.temperature { color: #ffee03; }
    .card.humidity { color: #03ffc4; }
    .card.pressure { color: #2dff03; }
    .card.gas { color: #ff00a6; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ELE 2036 - TEAM 8 | WEB SERVER</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> TEMPERATURE</h4><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> HUMIDITY</h4><p><span class="reading"><span id="hum">%HUMIDITY%</span> &percnt;</span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-angle-double-down"></i> PRESSURE</h4><p><span class="reading"><span id="pres">%PRESSURE%</span> hPa</span></p>
      </div>
      <div class="card gas">
        <h4><i class="fas fa-wind"></i> GAS</h4><p><span class="reading"><span id="gas">%GAS%</span> K&ohm;</span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
 
 source.addEventListener('pressure', function(e) {
  console.log("pressure", e.data);
  document.getElementById("pres").innerHTML = e.data;
 }, false);
 
 source.addEventListener('gas', function(e) {
  console.log("gas", e.data);
  document.getElementById("gas").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Init BME680 sensor
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();

//Initialize Display
  display.begin();

  // you can change the contrast around to adapt the display for the best viewing!
  display.setContrast(50);

  delay(2000);
  display.clearDisplay();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
    display.printf("T = %.2f \n", temperature);
    display.printf("H = %.2f \n", humidity);
    display.printf("P = %.2f \n", pressure); 
    display.printf("GasR= %.2f \n", gasResistance);
    display.println();
  display.display();
  delay(2000);
  display.clearDisplay();





  
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    getBME680Readings();
    Serial.printf("Temp = %.2f \n", temperature);
    Serial.printf("Humi = %.2f \n", humidity);
    Serial.printf("Pres = %.2f \n", pressure);
    Serial.printf("GasR = %.2f \n", gasResistance);
    Serial.println();

    //Display static text
      display.setContrast(50);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
    display.printf("T = %.2f \n", temperature);
    display.printf("H = %.2f \n", humidity);
    display.printf("P = %.2f \n", pressure); 
    display.printf("GasR= %.2f \n", gasResistance);
    display.println();
  display.display();
  delay(2000);
  display.clearDisplay();



    
    // Send Events to the Web Server with the Sensor Readings
    events.send("ping",NULL,millis());
    events.send(String(temperature).c_str(),"temperature",millis());
    events.send(String(humidity).c_str(),"humidity",millis());
    events.send(String(pressure).c_str(),"pressure",millis());
    events.send(String(gasResistance).c_str(),"gas",millis());
    
    lastTime = millis();
  }
}
