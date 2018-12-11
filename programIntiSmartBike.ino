
BERUBAAHHHHHH

#include <Wire.h>
//#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
#include <ArduinoJson.h>            //Protokol pengiriman data dari Internet ke Hardware
#include <string.h>                 //Keperluan Parsing data GPS
#include <SoftwareSerial.h>


ESP8266WiFiMulti WiFiMulti;

//init GPS
static const int RXPin = 4, TXPin = 3;
SoftwareSerial GPSss(RXPin, TXPin);
static const uint32_t GPSBaud = 4800;


//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
WiFiManager wifiManager;

//Deklarasi pin
#define BUZZER D5
#define SOLENOID D8
#define STATUSBATERAI A0


//Variabel untuk varsing data GPS
char input[] = "$GPRMC,061340.00,A,0746.32808,S,11022.51931,E,0.202,,031218,,,A*6F\n"; //Input
const char del[] = "$GPRMC,ASEF*M"; //Delimiters
char *token, *ok1, *ok2;
int i = 0;
char *array[10];  // Number of variables to save
int loc1, loc2;
const char str3 = '$';
const char str4 = '\n';


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  GPSss.begin(GPSBaud);
  inisialisasi();

}

void loop() {
  // put your main code here, to run repeatedly:

}

//Fungsi inisialisai
void inisialisasi() {

}
//Fungsi Membaca GPS

//Fungsi Timer booking

//Fungsi selenoid


//Fungsi Buzzer

//Fungsi Cek Jarak


//Fungsi Cek kondisi gembok

