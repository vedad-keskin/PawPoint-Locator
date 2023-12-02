#include <Wire.h> // LCD
#include <LiquidCrystal_I2C.h> // LCD
#include <MFRC522.h> // RFID
#include <SPI.h> // RFID
#include <TinyGPS++.h> //GPS
#include <SoftwareSerial.h> // Serial
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>


// -- LED LAMPICA
// D4 --> VECA 
// GND --> MANJA 

// -- LCD
// GND --> SIVA ---> GND 
// VCC --> LJUBICASTA --> VIN
// SDA --> PLAVA--> D2
// SCL --> ZELENA --> D1

// -- RFID
// SDA --> SMEDJA --> D4
// SCK --> CRNA --> D5
// MOSI --> BIJELA --> D7
// MISO --> SIVA --> D6
// GND --> CRNA --> GND
// RST --> LJUBICASTA --> D3
// 3.3V --> BIJELA --> 3.3.V

// -- GPS SENZOR
// VCC --> CRVENA --> 3V
// RX --> SMEDJA --> D1
// TX --> CRNA --> D2
// GND --> BIJELA --> GND


// - GPS Komponente
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps; // pravljenje gps 
SoftwareSerial ss(4, 5);  // primanje tx i rx 

// - LED Komponente
int led_pin = D0;

// -- RFID Komponente
#define SS_PIN D4  // RFID pinovi
#define RST_PIN D3  // RFID pinovi
 
MFRC522 rfid(SS_PIN, RST_PIN); // pokretanje RFID


String KodKojiSePoredi = "0 0 0 0"; // pocetni kod
String KodNotValidLjubimac1 = "179 70 210 207"; // Kod tag-a 
String KodValidLjubimac = "149 167 143 176"; // Kod tag-a 
String KodNotValidLjubimac3 = "36 179 143 201"; // Kod tag-a 
String KodNotValidLjubimac4 = "101 142 162 176"; // Kod tag-a 
String KodNotValidKartica1 = "51 57 75 168"; // Kod kartice

int brojac = 0;

String uidString;


// Firebase i Wifi postavke
//#define WIFI_SSID "Redmi 9A WPA2-Personal"
//#define WIFI_PASSWORD "zaurlajdvaput"

#define WIFI_SSID "IOT"
#define WIFI_PASSWORD "11111111"
#define API_KEY "AIzaSyBbEeluhEe-GpI_45hGxtUqyYF2NsnGLPQ"
#define DATABASE_URL "pawpoint-tracker-iot-default-rtdb.europe-west1.firebasedatabase.app" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;


// bool-ovi za firebase
bool LjubimacJeKuci = true;
bool ValidanLjubimac = false;
bool start = true;
float Latitude , Longitude;
String LatitudeString , LongitudeString;
int BrojacLED = 0;

void setup() {
  
  Serial.begin(9600);
  //lcd.begin(); // Paljenje LCD
  //lcd.backlight();  // Paljenje backlighta LCD 

  pinMode(led_pin, OUTPUT); // Paljenje LED
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());


  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("Nije prosla konekcija na firebase");
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  SPI.begin(); // pokretanje SPI
  rfid.PCD_Init(); // pokretanje RFID
  ss.begin(GPSBaud); // pokretanje gps-a
}


void loop() {

  //if (Firebase.RTDB.getInt(&fbdo, "Ljubimac/Blink")) {
    //  if (fbdo.dataType() == "int") {
      //  BrojacLED = fbdo.intData();
       
        //if(BrojacLED % 2 == 1){

          //digitalWrite(led_pin,HIGH);
          //delay(500);

          //digitalWrite(led_pin,LOW);  
          //delay(500);
      //  }
     // }
   // }

   if(start == true){
    Serial.println();
    Serial.print("Ljubimac nije skeniran");
    Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac nije skeniran");
   }
  if(KodKojiSePoredi != KodValidLjubimac && ValidanLjubimac == true && start == false){
    Serial.println();
    Serial.print(F("Ljubimac je kuci"));
    Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac je kuci");
    Firebase.RTDB.setInt(&fbdo, "Ljubimac/LjubimacJeKuci", 1);
  }  
  else if(ValidanLjubimac == false && start == false)
  {
    Serial.println();
    Serial.print(F("Ljubimac nije vas"));
    Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac nije vas");
    Firebase.RTDB.setString(&fbdo, "Ljubimac/ID", uidString);
    Firebase.RTDB.setInt(&fbdo, "Ljubimac/LjubimacJeKuci", 0);
  }
  else if(KodKojiSePoredi == KodValidLjubimac && ValidanLjubimac == true && start == false)
  {
     Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac je vani");

    //digitalWrite(led_pin,HIGH);
    //delay(500);

    //digitalWrite(led_pin,LOW);
    //delay(500);

    while (ss.available() > 0)
    if (gps.encode(ss.read()))
      PrikaziGPS();
  }


  if(rfid.PICC_IsNewCardPresent())
  {
      UcitajKarticu();
      start = false;
      delay(500);
  }
  
}

void UcitajKarticu(){


  rfid.PICC_ReadCardSerial();
  uidString = String(rfid.uid.uidByte[0])+" "+String(rfid.uid.uidByte[1])+" "+String(rfid.uid.uidByte[2])+ " "+String(rfid.uid.uidByte[3]);

  Serial.println("\n");
  Serial.println("Skenirana kartica ljubimca: ");
  Serial.println(uidString);
  Serial.println("\n");
  Firebase.RTDB.setString(&fbdo, "Ljubimac/ID", uidString);
  



  KodKojiSePoredi = uidString;
    
  if(KodKojiSePoredi == KodValidLjubimac){

    brojac++;
    ValidanLjubimac = true; // Ljubimac je registrovan
    LjubimacJeKuci = false;

    Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac je vani");
    Firebase.RTDB.setInt(&fbdo, "Ljubimac/LjubimacJeKuci", 0);
    Firebase.RTDB.setInt(&fbdo, "Ljubimac/ValidanLjubimac", 1);
  }
  if(KodKojiSePoredi == KodNotValidLjubimac1 || KodKojiSePoredi == KodNotValidLjubimac3 || KodKojiSePoredi == KodNotValidLjubimac4 || KodKojiSePoredi == KodNotValidKartica1){
     ValidanLjubimac = false;
     Firebase.RTDB.setInt(&fbdo, "Ljubimac/ValidanLjubimac", 0);
     Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac nije vas");
  }

  if(brojac==2){

    Serial.println();
    Serial.print(F("Ljubimac se vratio kuci"));
    Firebase.RTDB.setString(&fbdo, "Ljubimac/Poruka", "Ljubimac se vratio kuci");
    Firebase.RTDB.setInt(&fbdo, "Ljubimac/LjubimacJeKuci", 1);
    KodKojiSePoredi = "0 0 0 0";
    brojac=0;
  }
  
  
}






void PrikaziGPS()
{
  Serial.println("\n");
  Serial.print(F("Koordinate: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);

    Latitude = gps.location.lat();
    LatitudeString = String(Latitude , 6);
    Longitude = gps.location.lng();
    LongitudeString = String(Longitude , 6);

    Firebase.RTDB.setString(&fbdo, "Koordinate/GEOSirina", LatitudeString);
    Firebase.RTDB.setString(&fbdo, "Koordinate/GEODuzina", LongitudeString);
  }
  else
  {
    Serial.print(F("Trazi se GPS"));
  }

  Serial.print(F("  Datum: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());


    Firebase.RTDB.setInt(&fbdo, "Datum/Dan", gps.date.day());
    Firebase.RTDB.setInt(&fbdo, "Datum/Mjesec", gps.date.month());
    Firebase.RTDB.setInt(&fbdo, "Datum/Godina", gps.date.year());
    
    
  }
  else
  {
    Serial.print(F("Trazi se GPS"));
  }
  
  Serial.println();
}



