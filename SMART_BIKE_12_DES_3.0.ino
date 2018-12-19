/*
    SMART BIKE VERSI 1.4
    NUSANTERA TEKNOLOGI 2018
    15/12/2018 , 14.12 PM

*/
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = D3, TXPin = D4;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

const double STASIUN_LAT = -7.772092;
const double STASIUN_LNG = 110.375309;

#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#define USE_SERIAL Serial

WiFiManager wifiManager;
ESP8266WiFiMulti WiFiMulti;

//needed for library
int incomingByte = 0;
int WifiConnect = 0;
int initial = 0;
int waktuPinjam = 0;
int back = 0, back2 = 0;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String myStr;
int menit_selesai = 0;
int jam_selesai = 0;
int waktu_pinjam = 0;
int menit = 0;
bool buzzer = false;
bool getWaktu = true;
bool gps_status = false;
double distanceKm = 100000;

void setup(void)
{
  Serial.begin(9600);
  ss.begin(GPSBaud);

  // while(!Serial) continue;
  Wire.begin();
#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  delay(3000); // wait for console opening

  WiFi.mode(WIFI_AP_STA);
  WiFiMulti.addAP("majatech", "smartbike"); //SSID

  if (!WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("Wifi Fail");
    // Serial.flush();
    WifiConnect = 0;
    WifiConnect = 1;
  }

  pinMode(D2, OUTPUT); //buzzer
  pinMode(D8, OUTPUT); //solenoid

}


void loop(void) {

  while (ss.available() > 0)
    if (gps.encode(ss.read())) {
    //  checkJarak();
      Serial.println("[ ready GPS ]");
      gps_status = true;
    }
  // hitung_waktu(0,0);
  
  char *Str1 = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isBikeCondition&key=3289";
  char *StrTutupKunci = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isSet=1&key=3289&data=0";
  Serial.print(" [state : ]"); Serial.println(initial);
  Serial.print(" [waktu : ]"); Serial.println(waktuPinjam);
  Serial.print(" [ STATUS GPS : ] "); Serial.println(gps_status);

  switch (initial)
  {
    case 0:
      {
        getWaktu = true;
        back = checkInternet(Str1);
        if (back == 1 && gps_status == true) initial = 1;
        else initial = 0;

        if(buzzer == true)nyalaBuzzer();
       
        break;
      }
    case 1:
      {
        buzzer = false;
        bukaKunci();
        hitung_waktu(0, 0);
        initial = 2;
        break;
      }
    case 2:
      {
        back = tungguTimer();
        getWaktu = false;
        if (back == 1) {
          tutupKunci(StrTutupKunci);
          initial = 3;
        } else initial = 2;
        break;
      }
    case 3:
      {
        //while(1)

        buzzer = true;
        nyalaBuzzer();
        waktuPinjam = 0;
        back = checkInternet(Str1);
        back2 = checkJarak();
        if (back == 1) initial = 0;
        else if(distanceKm < 50) initial = 4;
        else initial = 5;
        break;
      }
    case 4:
      {
        tutupKunci(StrTutupKunci);
        waktuPinjam = 0;
        initial = 0;
        break;
      }

    case 5:
      {
        waktuPinjam = 0;
        initial = 0;
        break;
      }

  }    //delay(1000);
  yield();
}


void bukaKunci() {
  //place code here
  Serial.println("KEY OPENED!");
  digitalWrite(D8, HIGH);
  delay(1000);
  digitalWrite(D8, LOW);
  delay(1000);
  //GANTI STATE SELANJUTNYA

}


int hitung_waktu(int a, int b) {
  waktu_pinjam = waktuPinjam;
  menit = gps.time.minute() + waktu_pinjam;
  menit_selesai = menit % 60;
  jam_selesai = gps.time.hour() + (menit / 60);
  Serial.print("[ waktu sekarang ]" ); Serial.print(gps.time.hour()); Serial.print(" : "); Serial.println(gps.time.minute());
  Serial.print("[ waktu target ]" ); Serial.print(jam_selesai); Serial.print(" : "); Serial.println(menit_selesai);
  a = menit_selesai; b = jam_selesai;
}
int tungguTimer() {
  // place code here
  //GANTI STATE SELANJUTNYA
  checkJarak();
  Serial.print("[ JAM ] :"); Serial.print(jam_selesai); Serial.print(" :::: "); Serial.println(gps.time.hour());
  Serial.print("[ MENIT ] :"); Serial.print(menit_selesai); Serial.print(" :::: "); Serial.println(gps.time.minute());
  if (gps.time.hour() >= jam_selesai && gps.time.minute() >= menit_selesai) {
    Serial.print("sisa waktu : "); Serial.print(jam_selesai - gps.time.hour());
    Serial.print( " : "); Serial.println( menit_selesai - gps.time.minute());
    return 1;



  } else {
    return 0;
  }
  Serial.println("stop");

  //Serial.print(menit_selesai); Serial.print( " : "); Serial.println(jam_selesai);

  // return 0;




}

void nyalaBuzzer() {

  for (int i = 0; i < 25; i++ ) {
    digitalWrite(D2, HIGH);
    delay(200);
    digitalWrite(D2, LOW);
    delay(200);
  }
}

int checkJarak() {
  distanceKm = gps.distanceBetween( gps.location.lat(), gps.location.lng(), STASIUN_LAT, STASIUN_LNG) / 1000.0;
  Serial.print("Distance (km) to stasiun: ");
  Serial.println(distanceKm);
  return 0;
}

int tutupKunci(char * web) {
  // place code here
  //GANTI STATE SELANJUTNYA
  HTTPClient http;
  int httpCode;
  char *Str4;
  String payload;

  //program berjalan
  Serial.print("[HTTP] begin...\n");
  if (WiFiMulti.run() == WL_CONNECTED) {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    Str4 = web;
    Serial.print("API Size: "); Serial.println(sizeof(Str4));
    // configure traged server and url
    http.begin(Str4); //HTTP
    Serial.print("[HTTP] GET...\n");
    httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.println(payload);

      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

  }
  delay(1000);
}

int checkInternet(char *web)
{
  // place code here
  //deklarasi variable lokal
  StaticJsonBuffer<200> jsonBuffer;
  HTTPClient http;
  int httpCode;
  char *Str4;
  String payload;
  int data_status = 0;
  //String statusnya;
  //String peminjam;
  //String waktu;

  //program berjalan
  Serial.print("[HTTP] begin...\n");
  if (WiFiMulti.run() == WL_CONNECTED) {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    Str4 = web;
    //Serial.print("API Size: ");Serial.println(sizeof(Str4));
    // configure traged server and url
    http.begin(Str4); //HTTP
    Serial.print("[HTTP] GET...\n");
    httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.println(payload);

        //char json[] = "{\"statusnya\":1,\"peminjam\":\"karna\",\"waktu pinjam\":60}";
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (!root.success()) {
          Serial.println("parseObject() failed");
          return 0;
        }

        String statusnya = root["status"];
        String peminjam = root["peminjam"];
        String waktu = root["waktu pinjam"];

        Serial.print("status : "); Serial.println(statusnya);
        Serial.print("peminjam : "); Serial.println(peminjam);
        Serial.print("waktu : "); Serial.println(waktu);

        // STATE SELANJUTNYA
        if (statusnya == "1") {
          // initial=1;
          waktuPinjam = 2; //waktu.toInt();
          data_status = 1;
        } else {
          data_status = 0;
        }

      }
    } else {
      Serial.printf("[HTTP] GET... tidak connect, error: %s\n", http.errorToString(httpCode).c_str());
      return 0;
    }
    http.end();

  }
  delay(1000);

  return data_status;
}
