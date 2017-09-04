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

// Various constants/variables for the Temperature/Humidity Sensor
#define DHTPIN            7         // Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT11     // DHT 11

// Various constants for the Ultrasonic Sensors
#define US1ECHO           3
#define US1TRIG           4
#define US2ECHO           5
#define US2TRIG           6

// EtherCard MAC Address and IP configuration
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = {192,168,1,250};
static byte mygw[] = {192,168,1,1};
static byte mydns[] = {192,168,1,1};

// Constants for the LED pins
#define GREENLED          8
#define REDLED            9