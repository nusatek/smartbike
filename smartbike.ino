//Deskripsi   : Program ini sudah saya uji dengan menggunakan Arduino Uno
//              Power supply untuk SIM800L saya gunakan ower supply dari luar
//              dengan adanya switching regulator dengan keluaran 3,8 - 4V
//              Status "1" : Permintaan akses NusaBike
//

#include <ArduinoJson.h>          //Untuk SIM800L
#include <Http.h>                 //Untuk SIM800L
#include <TinyGPS++.h>            //Untuk GPS
#include <SoftwareSerial.h>       //Untuk GPS



//Parameter dan variabel dari SIM800L
unsigned int RX_PIN = 9;
unsigned int TX_PIN = 8;
unsigned int RST_PIN = 12;
HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);

//Parameter dan variabel dari GPS
static const int RXPin = 3, TXPin = 2;      // GPS RX <==> TXPin Arduino    GPS TX <==> RXPin Arduino (pin3)
static const uint32_t GPSBaud = 9600;       //Baudrate GPS (dapat diubah)
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
SoftwareSerial ssGPS(RXPin, TXPin);

//cek counter gps
int counts_gps = 0;
#define MAX_COUNT 10                //Pengulangan sebanyak 10 detik 

//////////////////////////////////////
//Parameter dan variable Buzzer
#define BUZZER 5

//////////////////////////////////////
//Parameter dan variable drive selenoid
#define SELENOID 7
//////////////////////////////////////
//Parameter dan variable limit switch
#define LIMSWITCH 6
int kunciStatus = 0;  //terbuka = 0, tertutup  = 1

////////////////////////////////////
//Pin indikator
#define INDIKATOR1 4
#define INDIKATOR2 10
#define INDIKATOR3 11
#define INDIKATOR4 13

////////////////////////////////////
//Parameter Voltage Sensor
#define VOLSENSOR A0

////////////////////////////////////
//Parameter pada fungsi switch
int initial = 0;          //Variabel yang menunjukkan status 0/1
int back = 0, back2 = 0;
int sisaWaktu = 0;
int waktuPinjam = 0;
int menit_selesai = 0;
int jam_selesai = 0;
int waktu_pinjam = 0;
int menit = 0;
int cnt = 0;

//batre baca
int persenVolt = 0;
char *web;

unsigned long lastRunTime = 0;
unsigned long waitForRunTime = 0;

void setup() {
  Serial.begin(9600);             //Baudrate komunikasi mikrokontroler dengan PC

  ssGPS.begin(GPSBaud);                   //Baudrate GPS 9600 bps
  gps_init();                             //Menginisialisasi GPS
  pinMode(INDIKATOR1, OUTPUT);
  pinMode(INDIKATOR2, OUTPUT);
  pinMode(INDIKATOR3, OUTPUT);
  pinMode(INDIKATOR4, OUTPUT);
  digitalWrite(INDIKATOR1, LOW);
  digitalWrite(INDIKATOR2, LOW);
  digitalWrite(INDIKATOR3, LOW);
  digitalWrite(INDIKATOR4, LOW);

  pinMode(BUZZER, OUTPUT);        //Mode pin D5 Sebagai output untuk mengendalikan BUZZER
  pinMode(SELENOID, OUTPUT);      //Mode pin D6 Sebagai output untuk mengendalikan SELENOID
  digitalWrite(SELENOID, LOW);
  pinMode(LIMSWITCH, INPUT_PULLUP);      //Mode pin D7 Sebagai INPUT untuk mengendalikan LIMIT SWITCH
  pinMode(VOLSENSOR, INPUT);       //
  while (!Serial);
  Serial.println("Starting!");


}

void loop() {
  Serial.println("-------------------------------------");
  Serial.print("[ STATE ] ");
  Serial.println(initial);
  Serial.println("-------------------------------------");

  while (ssGPS.available() > 0) {
    if (gps.encode(ssGPS.read())) {
      //displayInfo();
      hitung_waktu(0, 0);
    }
  }

  switch (initial) {
    case 0:                           //Kondisi stand by Mode
      {
        setIndikator(0);
        if (shouldTrackTimeEntry()) {
          web = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isBikeCondition&key=3289&volt=200";
          initial = SendInternet(web, 1);
          Serial.println("+-------------------------");
        }
        //Membaca kapasitas batrai dan kirim
        break;
      }

    case 1:
      {
        setIndikator(0);
        bukaKunci(2000);                  //Membuka Kunci
        initial = 2;                  //Pindah ke case 2
        break;
      }

    case 2:
      {
        setIndikator(1);
        sisaWaktu = menjalankanTimer();   //Melakukan kalkulasi sisa waktu pemakaian
        if (sisaWaktu == 1) {
          web = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isSet=1&key=3289&data=0";
          SendInternet(web, 2);
          initial = 3;                    //Pindah ke case 3
        }
        break;
      }

    case 3:
      {
        int data;
        setIndikator(2);
        buzzerNyala(150, 10);
        cnt++;
        if(cnt ==5) {
           web = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isBikeCondition&key=3289&volt=200";
           data = SendInternet(web, 1);
           cnt = 0;
        }
         int data2 = tutupKunci();
         if(data2 == 1) initial =4;
         else initial = 3;
          
        break;
      }
    case 4:
      {
        setIndikator(3);
        web = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isSet=1&key=3289&data=0";
        SendInternet(web, 2);
        initial = 0;
        break;
      }

  }


}

void print(const __FlashStringHelper *message, int code = -1) {
  if (code != -1) {
    Serial.print(message);
    Serial.println(code);
  }
  else {
    Serial.println(message);
  }
}

bool shouldTrackTimeEntry() {
  // This calculation uses the max value the unsigned long can store as key. Remember when a negative number
  // is assigned or the maximun is exceeded, then the module is applied to that value.
  unsigned long elapsedTime = millis() - lastRunTime;
  print(F("Elapsed time: "), elapsedTime);
  return elapsedTime >= waitForRunTime;
}

// functions
void setIndikator(int indik_set) {
  int data_pin[4] = {INDIKATOR1, INDIKATOR2, INDIKATOR3, INDIKATOR4};
  for(int i = 0; i < 4; i++) {
    if(indik_set == i) {
      digitalWrite(data_pin[i], HIGH);
    }else {
      digitalWrite(data_pin[i], LOW);
    }
  }
}

void gps_init() {
  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void displayInfo() {
  Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

void cekGPS() {
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    counts_gps = 0;
  }
  else {
    Serial.print(F("INVALID"));
    counts_gps++;
    Serial.print(" [ BATAS COUNTER CHECK : ] "); Serial.println( counts_gps);
  }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Menyalakan buzzer
void buzzerNyala(int jeda, int hingga) {
  for(int i = 0; i < hingga; i++) {
    digitalWrite(BUZZER, HIGH);
    Serial.println(F("Buzzer ON"));
    delay(jeda);
    digitalWrite(BUZZER, LOW);
    Serial.println(F("Buzzer OFF"));
    delay(jeda);
  }
  //}else {
   // cnt=0;
  //}
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Membuka Gembok
int bukaKunci(int jeda) {
  digitalWrite(SELENOID, HIGH);
  Serial.println("SELENOID ON");
  delay(jeda);
  digitalWrite(SELENOID, LOW);
  Serial.println("SELENOID OFF");
  //delay(jeda);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Membuka Gembok
int tutupKunci() {
  kunciStatus = digitalRead(LIMSWITCH);
  if (kunciStatus == 0) {
    return 1;
  } else {
    return 0;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Mengecek waktu peminjaman
int hitung_waktu(int a, int b) {
  waktu_pinjam = waktuPinjam;
  menit = gps.time.minute() + waktu_pinjam;
  menit_selesai = menit % 60;
  jam_selesai = gps.time.hour() + (menit / 60);
  Serial.print("[ waktu sekarang ]" ); Serial.print(gps.time.hour()); Serial.print(" : "); Serial.println(gps.time.minute());
  Serial.print("[ waktu target ]" ); Serial.print(jam_selesai); Serial.print(" : "); Serial.println(menit_selesai);
  a = menit_selesai; b = jam_selesai;
}

int menjalankanTimer() {
  hitung_waktu(0, 0);

  if (gps.time.hour() >= jam_selesai && gps.time.minute() >= menit_selesai) {
    return 1;
  } else {
    Serial.print(menit_selesai); Serial.print( " : "); Serial.println(jam_selesai);
    return 0;
  }
}
///////////////////////////////////////////////////////////////
//Fungsi membaca tegangan batrai
int setupAwal() {
  float baca = analogRead(VOLSENSOR);
  float baca2 = (((baca / 1024) * 8.4) + 0.7);
  persenVolt = ((baca2 - 7) / (8.4 - 7)) * 100;
  Serial.print("Voltage :");
  Serial.println(baca2);
  Serial.print("persen : "); Serial.println(persenVolt);
  //trackTimeEntrySend(hasil);         //Mengirim kapasitas power batrai tersi
  delay(100);
  String str = "http://bike.majapahitech.com/apiv2/main/smartbike.php?isBikeCondition&key=3289&volt=";
  str += String(persenVolt);
  //Serial.println(str);
  char charBuf[1000];
  str.toCharArray(charBuf, 1000);
  Serial.println(charBuf);
  SendInternet(charBuf, 2);
}
int SendInternet(char * str, int flag) {
  char response[32];
  char body[90];
  int status_p = 0;
  Result result;
  print(F("Cofigure bearer: "), http.configureBearer("internet"));
  result = http.connect();
  print(F("HTTP connect: "), result);

  result = http.get(str, response);
  print(F("HTTP GET: "), result);
  if (result == SUCCESS) {
    if (flag == 1) {
      Serial.println(response);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(response);
      String statusnya = root["s"];
      //String peminjam = root["p"];
      String waktu = root["w"];

      if (statusnya == "1") {
        status_p = 1;
        waktuPinjam = 0;//waktu.toInt();
      }else {
        status_p = 0;
      }

      Serial.print("status : "); Serial.println(statusnya);
      //    Serial.print("peminjam : "); Serial.println(peminjam);
      Serial.print("waktu : "); Serial.println(waktu);
      /////////////////
      lastRunTime = millis();
      waitForRunTime = root["waitForRunTime"];

      print(F("Last run time: "), lastRunTime);
      print(F("Next post in: "), waitForRunTime);
    
    }else if (flag == 2) {
      Serial.println(response);
      status_p = 0;
    }

  }

  print(F("HTTP disconnect: "), http.disconnect());
  return status_p;
}
