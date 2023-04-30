#define BLYNK_TEMPLATE_ID "TMPLqPIMnzdC"
#define BLYNK_TEMPLATE_NAME "RelayLDR"
#define BLYNK_AUTH_TOKEN "VXoUYLg4N1nocjCsKNP4XoTa0cNaHYUA"

const char* ssid = "androidAP";
const char* password = "12345678";

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h> 
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Preferences.h>
#include <AceButton.h>
#include <SPI.h>
#include <Wire.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>     

using namespace ace_button;
Preferences pref;

#define DHTTYPE DHT22   
#define DHTPIN     2
#define LDR_PIN   A0
#define RelayPin1 5  //D1
#define RelayPin2 4  //D2
#define RelayPin3 14
#define RelayPin4 12

#define ButtonPin1 10 //D25
#define ButtonPin2  9  //D26 
#define ButtonPin3 13  //D27
#define ButtonPin4  3

#define wifiLed   16   //D2


#define VPIN_Text           V0
#define VPIN_currentTemp    V1
#define VPIN_currentHumi    V2
#define VPIN_Relay1         V3
#define VPIN_Relay2         V4
#define VPIN_Relay3         V5
#define VPIN_LuxRelay4      V6
#define VPIN_currentLDR     V7
#define VPIN_setLdrvalmax   V8
#define VPIN_setLdrvalmin   V9


float currentTemp = 0;
float currentHumi = 0;
float ldrVal = 0;



// Relay and Mode State
bool Relay1State = LOW; 
bool Relay2State = LOW; 
bool Relay3State = LOW;
bool LuxRelay4State = LOW;

int wifiFlag = 0;

// const int maxRelay3 = 700;
 //const int minRelay3 = 200;

float maxLight = 0;
float minLight = 0;


DHT dht(DHTPIN, DHTTYPE);
char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;





ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
ButtonConfig config4;
AceButton button4(&config4);


void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
void handleEvent4(AceButton*, uint8_t, uint8_t);


// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_Relay1) {
  Relay1State = param.asInt();
  digitalWrite(RelayPin1, !Relay1State);
  pref.putBool("Relay1", Relay1State);
}

BLYNK_WRITE(VPIN_Relay2) {
  Relay2State = param.asInt();
  digitalWrite(RelayPin2, !Relay2State);
  pref.putBool("Relay2", Relay2State);
}

BLYNK_WRITE(VPIN_Relay3) {
  Relay3State = param.asInt();
  digitalWrite(RelayPin3, !Relay3State);
  pref.putBool("Relay3", Relay3State);
}



BLYNK_WRITE(VPIN_LuxRelay4) {
  LuxRelay4State = param.asInt();
  digitalWrite(RelayPin4, !LuxRelay4State);
  pref.putBool("LuxRelay4", LuxRelay4State);
}




BLYNK_WRITE(VPIN_setLdrvalmin) {
  minLight = param.asFloat();
  pref.putBool("setmin", minLight);
}

BLYNK_WRITE(VPIN_setLdrvalmax) {
  maxLight = param.asFloat();
  pref.putBool("setmax", maxLight);
}

void checkBlynkStatus() { // called every 2 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    
    digitalWrite(wifiLed, HIGH);
    Blynk.virtualWrite(VPIN_Text, "IoT Temperature & Humidity Controller");
  }
  
  delay(1000);
}

BLYNK_CONNECTED() {
  // update the latest state to the server
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature & Humidity Controller");
  
  Blynk.syncVirtual(VPIN_currentTemp);
  Blynk.syncVirtual(VPIN_currentHumi);
  Blynk.syncVirtual (VPIN_currentLDR);
  Blynk.syncVirtual(VPIN_setLdrvalmin);
  Blynk.syncVirtual(VPIN_setLdrvalmax);
  Blynk.virtualWrite(VPIN_Relay1, Relay1State);
  Blynk.virtualWrite(VPIN_Relay2, Relay2State);
  Blynk.virtualWrite( VPIN_Relay3, Relay3State); 
  Blynk.virtualWrite( VPIN_LuxRelay4, LuxRelay4State);
  
       
}

void setup()
{
  
  Serial.begin(115200);
  //Open namespace in read-write mode
  pref.begin("Relay_State", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  pinMode(ButtonPin1, INPUT_PULLUP);
  pinMode(ButtonPin2, INPUT_PULLUP);
  pinMode(ButtonPin3, INPUT_PULLUP);
  pinMode(ButtonPin4, INPUT_PULLUP);

  
  
  
  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, !Relay1State);
  digitalWrite(RelayPin2, !Relay2State);
  digitalWrite(RelayPin3, !Relay3State);
  digitalWrite(RelayPin4, !LuxRelay4State);

  
  dht.begin();    // Enabling DHT sensor
  digitalWrite(wifiLed, LOW);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
  config4.setEventHandler(button4Handler);
  
  
  button1.init(ButtonPin1);
  button2.init(ButtonPin2);
  button3.init(ButtonPin3);
  button4.init(ButtonPin4);
  
  
 Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    delay(5000);{
    WiFiManager wifiManager;
    wifiManager.autoConnect("BLYNK-IOT");
    Serial.println(WiFi.localIP());  
    }
   WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  //Blynk.begin(auth, ssid, pass);
 
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(1000);

  getRelayState(); // Get the last state of Relays and Set values of Temp & Humidity

  Blynk.virtualWrite(VPIN_Relay1, Relay1State);
  Blynk.virtualWrite(VPIN_Relay2, Relay2State);
  Blynk.virtualWrite(VPIN_Relay3, Relay3State); 
  Blynk.virtualWrite(VPIN_LuxRelay4, LuxRelay4State);
  Blynk.virtualWrite(VPIN_setLdrvalmin, minLight);
  Blynk.virtualWrite(VPIN_setLdrvalmax,  maxLight); 
 
 }


void readSensor() {
  ldrVal = analogRead(LDR_PIN);
  ldrVal = map(ldrVal, 0, 1023, 0, 100);
  Serial.print("LDR - "); Serial.println(ldrVal);
  delay(2000);
  currentTemp = dht.readTemperature();
  Serial.print("Temp - "); Serial.println(currentTemp);
  delay(2000);
  currentHumi = dht.readHumidity();
  Serial.print("Humi - "); Serial.println(currentHumi);
  delay(2000);
  if (isnan(currentHumi) || isnan(currentTemp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void sendSensor()
{
  readSensor();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature & Humidity Controller");
  Blynk.virtualWrite(VPIN_currentTemp, currentTemp);
  Blynk.virtualWrite(VPIN_currentHumi, currentHumi);
  Blynk.virtualWrite(VPIN_currentLDR, ldrVal);
  
}

void getRelayState()
{
  //Serial.println("reading data from NVS");
  
  Relay1State = pref.getBool("Relay1", 0);
  digitalWrite(RelayPin1, !Relay1State);
  Blynk.virtualWrite(VPIN_Relay1, Relay1State);
  delay(200);
  
  Relay2State = pref.getBool("Relay2", 0);
  digitalWrite(RelayPin2, !Relay2State);
  Blynk.virtualWrite(VPIN_Relay2, Relay2State);
  delay(200);

  Relay3State = pref.getBool("Relay3", 0);
  digitalWrite(RelayPin3, !Relay3State);
  Blynk.virtualWrite(VPIN_Relay3, Relay3State);
  delay(200); 
  
  LuxRelay4State = pref.getBool("LuxRelay4", 0);
  digitalWrite(RelayPin4, !LuxRelay4State);
  Blynk.virtualWrite(VPIN_LuxRelay4, LuxRelay4State);
  delay(200);
  
  
  minLight = pref.getBool("setmin", 0);
  Blynk.virtualWrite(VPIN_setLdrvalmin, minLight);
  delay(200);
  

 maxLight = pref.getBool("setmax", 0);
  Blynk.virtualWrite(VPIN_setLdrvalmax, maxLight);
  delay(200);



}





void loop()
{





  
   //LDR control Relay 2
      if( ldrVal < minLight){
        if(LuxRelay4State == 0){
          digitalWrite(RelayPin4,LOW ); // turn ON relay 2
          LuxRelay4State = 1;
          // Update Button Widget
          Blynk.virtualWrite(VPIN_LuxRelay4, LuxRelay4State);
        }
      }
      else if (ldrVal > maxLight){
        if(LuxRelay4State == 1){
           digitalWrite(RelayPin4,HIGH ); // turn OFF relay 2
           LuxRelay4State = 0;
           // Update Button Widget
           Blynk.virtualWrite(VPIN_LuxRelay4, LuxRelay4State);
          }
      } 
    
  
  
  Blynk.run();
  timer.run();
 
  button1.check();
  button2.check();
  button3.check();
  button4.check();
 

}


void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Relay1");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin1, Relay1State);
      Relay1State = !Relay1State;
      pref.putBool("Relay1", Relay1State);
      Blynk.virtualWrite(VPIN_Relay1, Relay1State);
      break;
  }
}
void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Relay2");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin2, Relay2State);
      Relay2State = !Relay2State;
      pref.putBool("VPIN_Relay2", Relay2State);
      Blynk.virtualWrite(VPIN_Relay2, Relay2State);
      break;
  }
}



void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Relay3");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin4, Relay3State);
      Relay3State = !Relay3State;
      pref.putBool("VPIN_Relay3", Relay3State);
      Blynk.virtualWrite(VPIN_Relay3, Relay3State);
      break;
  }
}
void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("LuxRelay4");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin4, LuxRelay4State);
      LuxRelay4State = !LuxRelay4State;
      pref.putBool("VPIN_LuxRelay4", LuxRelay4State);
      Blynk.virtualWrite(VPIN_LuxRelay4, LuxRelay4State);
      break;
  }
}
