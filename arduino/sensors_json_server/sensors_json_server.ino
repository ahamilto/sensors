// Sensors JSON Server
// MIT License
// Copyright (c) 2017 Andrew Hamilton

// Arduino Pin Allocations
// 0,1 - Serial I/O
// 2 - Ethernet Shield
// 3,4 - Ultrasonic 1 (echo,trigger)
// 5,6 - Ultrasonic 2 (echo,trigger)
// 7 - Temperature/Humidity Sensor
// 8 - Green LED
// 9 - Red LED
// 10 - Ethernet Shield
// 11,12,13 - ICSP header/Ethernet Shield

// Debugging help
// #define DEBUG 1
#ifdef DEBUG
  #define DEBUGPRINT(x) Serial.println(x)
#else
  #define DEBUGPRINT(x)
#endif

// Libraries for the Temperature/Humidity Sensor
// https://github.com/adafruit/Adafruit_Sensor
// https://github.com/adafruit/DHT-sensor-library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Library for the EtherCard shield
// https://github.com/jcw/ethercard
#include <EtherCard.h>

#include "sensors_json_server.h"

//DHT Sensor Object
DHT_Unified dht(DHTPIN, DHTTYPE);

//EtherCard buffer space
byte Ethernet::buffer[500];
BufferFiller bfill;

void setup() {
  Serial.begin(9600);
  // Initialize the LEDs first
  pinMode(GREENLED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  
  // Set both LEDs ON during initialization
  digitalWrite(GREENLED, HIGH);
  digitalWrite(REDLED, HIGH);
  
  // Initialize the temperature sensor.
  dht.begin();
  Serial.println("Initializing DHTxx Unified Sensor");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.println("------------------------------------");

  Serial.println("\n[Starting the Arduino Server]");
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  Serial.println();
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  Serial.println("Configuring static IP...");
  ether.staticSetup(myip, mygw, mydns);
  
  ether.printIp("IP: ", ether.myip);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
  ether.printIp("Surf to this IP to access the sensor data: ", ether.myip);

  //Done initializing; turn off Red LED
  digitalWrite(REDLED, LOW);
}

float readTemp() {
  // Get the temperature event and return the temperature value
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    DEBUGPRINT("Error reading temperature!");
  }
  else {
    DEBUGPRINT("Read Temperature");
    DEBUGPRINT(event.temperature);
    DEBUGPRINT(" *C");
    return event.temperature;
  }
  return (float)-1;
}

float readHumidity() {
  // Get the temperature event and return the temperature value
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    DEBUGPRINT("Error reading humidity!");
  }
  else {
    DEBUGPRINT("Read Humidity");
    DEBUGPRINT(event.relative_humidity);
    return event.relative_humidity;
  }
  return (float)-1;
}

// This function will trigger an Ultrasonic Sensor and return the duration of the pulse
long readRangeFinder(int trig, int echo) {
  long result;
  // The PING is triggered by a HIGH pulse of 2 or more microseconds on the trigger pin.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(2);
  digitalWrite(trig, LOW);

  // The echo pin is used to read the signal from the PING; a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echo, INPUT);
  result = pulseIn(echo, HIGH);

  // Return the duration of the pulse as the result.
  return result;
}

long microsecondsToInches(long microseconds) {
  // The speed of sound is 1130 feet/second which translates to roughly
  // 74 microseconds per inch. The ping time is round-trip so we divide by two
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

static word jsonData() {
  // Turn on the Red LED for the duration of this operation
  digitalWrite(REDLED, HIGH);

  // Get the system uptime in seconds
  long t = millis() / 1000;
  //word h = t / 3600;
  //byte m = (t / 60) % 60;
  //byte s = t % 60;

  // Fetch the temperature and humidity
  char tempstr[15];
  char humiditystr[15];
  dtostrf(readTemp(), 5, 2, tempstr);
  dtostrf(readHumidity(), 5, 2, humiditystr);

  // Read the two Ultrasonic sensors
  long duration1;
  long inches1;
  duration1 = readRangeFinder(US1TRIG, US1ECHO);
  //duration1 = readRangeFinder(4, 3);
  inches1 = microsecondsToInches(duration1);

  long duration2, inches2;
  duration2 = readRangeFinder(US2TRIG, US2ECHO);
  //duration2 = readRangeFinder(6, 5);
  inches2 = microsecondsToInches(duration2);

  // Format the JSON response
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\n"
    "Content_Type: application/json\n"
    "Pragma: no-cache\n"
    "\n"
    "{\n"));
  bfill.emit_p(PSTR(
    "\"uptime\": \"$D\",\n"),
    t);
  bfill.emit_p(PSTR(
    "\"temperature\": \"$S\",\n"
    "\"humidity\": \"$S\",\n"),
    tempstr, humiditystr);
  bfill.emit_p(PSTR(
    "\"pulse duration 1\": \"$D\",\n"),
    duration1);
  bfill.emit_p(PSTR(
    "\"inches 1\": \"$D\",\n"),
    inches1);
  bfill.emit_p(PSTR(
    "\"pulse duration 2\": \"$D\",\n"),
    duration2);
  bfill.emit_p(PSTR(
    "\"inches 2\": \"$D\"\n"),
    inches2);
  bfill.emit_p(PSTR("}\n"));

  // Turn off the Red LED
  digitalWrite(REDLED, LOW);
  
  return bfill.position();
}

void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) {
    ether.httpServerReply(jsonData());
  }
}
