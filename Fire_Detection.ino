#include "credentials.h"

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

#include <Servo.h>
#include <DHT.h>

#include <string.h>

// Wifi & Blynk Credentials are located at credentials.h

// Define PIN
#define FLAME_SENSOR_PIN 33
#define BUZZER_PIN 25

#define LED_RED_PIN 26
#define LED_GREEN_PIN 

#define SERVO_PIN 27

#define DHT_PIN 32
#define DHTTYPE DHT11

#define FLAME_SENSOR_PIN
#define RELAY_PIN

// Variables
DHT dht(DHT_PIN, DHTTYPE);
Servo servo;
BlynkTimer timer;

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

  // if (!isnan(temperature) && !isnan(humidity)) {
  //   Blynk.virtualWrite(V1, temperature); // Send Temperature
  //   Blynk.virtualWrite(V2, humidity);    // Send Humidity
  
  if (temperature != 0.0 && humidity != 0.0) {
    Blynk.virtualWrite(V1, temperature); // Send Temperature
    Blynk.virtualWrite(V2, humidity);    // Send Humidity

  } else{
    Serial.println("## temp/humidity null");

  }
}

// Servo Control
BLYNK_WRITE(V3) {
  int position = param.asInt();
  servo.write(position);
}

// Relay control
void pumpControl(String command){
  if(strcmp(RELAY_PIN, "ON")){
    digitalWrite(RELAY_PIN, HIGH);

  } else if(strcmp(command, "OFF")){
    digitalWrite(PIN, LOW);

  }
}

// Logic
void mainLogic(){

  if(flameState == 1){
    Serial.println("Fire detected");
    pumpControl("ON");

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    Blynk.virtualWrite(V5, 1); // LED Indicator ON
    Blynk.virtualWrite(V4, 1); // Buzzer ON

  } else{
    Serial.println("No Fire Detected");
    pumpControl("OFF");

    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Blynk.virtualWrite(V5, 0); // LED Indicator OFF
    Blynk.virtualWrite(V4, 0); // Buzzer OFF

  }
}

void checkWifi(){
  Serial.print("Wifi Status: ")
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Connected!")
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  } else{
    Serial.println("Disconnected!");

  }
}

void logAll(){
  Serial.println("+-----------------------+");
  Serial.println("| Log all functionality |");
  Serial.println("+-----------------------+");

  Serial.print("Flame state: ");
  Serial.println(flameState)

  Serial.print("Temp: ");
  Serial.println(temperature);

  Serial.print("Humid: ");
  Serial.println(humidity);
  
  checkWifi();

  Serial.print("Blynk: ")
  if(Blynk.connected()){
    Serial.println("Connected");
  } else{
    Serial.println("Disconnected");
  }

  delay(4000);

}

void setup() {
  Serial.begin(115200);

  // INPUT
  pinMode(FLAME_SENSOR_PIN, INPUT);
  dht.begin();

  // OUTPUT
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(BUILTIN_LED, OUTPUT);

  servo.attach(SERVO_PIN);

  // Connect to blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  checkWifi();

  // Timers for sending data
  timer.setInterval(1000L, sendFlameData); // Flame Sensor every 1 sec
  timer.setInterval(2000L, sendDHTData);   // DHT Data every 2 sec

}

void loop() {
  Blynk.run();
  timer.run();

  mainLogic(); 
  logAll();

}
