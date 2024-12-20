// Wifi & Blynk Credentials are located in credentials.h
#define BLYNK_TEMPLATE_ID CRED_BLYNK_T_ID
#define BLYNK_TEMPLATE_NAME CRED_BLYNK_T_NAME
#define BLYNK_AUTH_TOKEN CRED_BLYNK_AUTH_TOKEN
#define BLYNK_PRINT Serial

#include "credentials.h"
#include "pins_arduino.h"

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

#include <Servo.h>
#include <DHT.h>

#include <string.h>

// Define PIN
//INPUT
#define FLAME_SENSOR_PIN D11
#define DHT_PIN D12
#define DHTTYPE DHT11

//OUTPUT
#define BUZZER_PIN D4
#define LED_RED_PIN D8
#define LED_GREEN_PIN D9

//ACTUATOR-OUTPUT
#define SERVO_PIN D4 // need pwm
#define RELAY_PIN D3

// Variables
DHT dht(DHT_PIN, DHTTYPE);
Servo servo;
BlynkTimer timer;

const char WIFI_SSID[] = CRED_WIFI_SSID;
const char WIFI_PASS[] = CRED_WIFI_PASS;

int flameState = 0;
float temperature = 0.0;
float humidity = 0.0;

// Flame Sensor Data
void sendFlameData() {
  flameState = digitalRead(FLAME_SENSOR_PIN);
  Blynk.virtualWrite(V0, flameState);
  
}

// DHT11 Data
void sendDHTData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(V1, temperature); // Send Temperature
    Blynk.virtualWrite(V2, humidity);    // Send Humidity
  } 
}

// Servo Control
BLYNK_WRITE(V3) {
  unsigned short position = param.asInt();
  servo.write(position);
}

// Relay control
void pumpControl(const char *command){
  if (strcmp(command, "ON") == 0) {
    digitalWrite(RELAY_PIN, LOW);

  } else if (strcmp(command, "OFF") == 0) {
    digitalWrite(RELAY_PIN, HIGH);

  }
}


void buzzerControl(){


}

// override test
BLYNK_WRITE(V9){
  flameState = param.asInt();
}

// Logic
void mainLogic(){
  if(flameState == 1){
    Serial.println("Fire detected");
    pumpControl("ON");

    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    Blynk.virtualWrite(V5, 1); // LED Indicator ON
    Blynk.virtualWrite(V4, 1); // Buzzer ON

  } else{
    Serial.println("No Fire Detected");
    pumpControl("OFF");

    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Blynk.virtualWrite(V5, 0); // LED Indicator OFF
    Blynk.virtualWrite(V4, 0); // Buzzer OFF

  }
  
}

void wifiConnect(){
  Serial.println("Connecting to WiFi!");
  WiFi.begin(WIFI_SSID, WIFI_PASS); 

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }
  Serial.println();
  wifiCheck();

}

void wifiCheck(){
  if(WiFi.status() == WL_CONNECTED){
    Serial.println(F("Connected"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());

  } else{
    Serial.println(F("Disconnected"));
    wifiConnect();

  }
}

void connectionCheck(){
  if(!(Blynk.connected())){
    Serial.println(F("Disconnected"));
    Serial.println(F("Trying to connect to Blynk Server"));
   
    while (Blynk.connect(3333) == false) {
      Serial.print(F("."));

    }
  } else{
    Serial.println(F("Connected"));  

  }
}

void logAll(){
  Serial.println(F("\n+-----------------------+"));
  Serial.println(F("| Log all functionality |"));
  Serial.println(F("+-----------------------+"));

  Serial.print(F("Flame state: "));
  Serial.println(flameState);

  Serial.print(F("Temp: "));
  Serial.println(temperature);

  Serial.print(F("Humid: "));
  Serial.println(humidity);
  
  Serial.print(F("Wifi Status: "));
  wifiCheck();

  Serial.print(F("Blynk Status: "));
  connectionCheck();

}

void setup() {
  Serial.begin(115200);

  // INPUT
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(DHT_PIN, INPUT);
  dht.begin();

  // OUTPUT
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);

  servo.attach(SERVO_PIN);

  //Connect to wifi
  wifiConnect();

  //Connect to blynk
  Blynk.config(BLYNK_AUTH_TOKEN); 
  connectionCheck();

  // Timers for sending data
  timer.setInterval(3000L, sendFlameData);
  // timer.setInterval(3000L, sendDHTData);   
  timer.setInterval(5000L, logAll); // Print all sensor and functionalities status to serial every 5 sec

  // run main logic
  timer.setInterval(1500L, mainLogic);

}

void loop() {
  Blynk.run();
  timer.run();
  
}
