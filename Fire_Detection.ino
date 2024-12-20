#include "credentials.h"
#include "pins_arduino.h"

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

#include <Servo.h>
#include <DHT.h>

#include <string.h>

// Wifi & Blynk Credentials are located at credentials.h

// Define PIN
#define FLAME_SENSOR_PIN D6
#define BUZZER_PIN D5

#define LED_RED_PIN D3
#define LED_GREEN_PIN D4

#define DHT_PIN D8
#define DHTTYPE DHT11

#define SERVO_PIN D7
#define RELAY_PIN D9

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

  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(V1, temperature); // Send Temperature
    Blynk.virtualWrite(V2, humidity);    // Send Humidity
  } else {
    Serial.println(F("## temp/humidity null"));

  }
}

// Servo Control
BLYNK_WRITE(V3) {
  int position = param.asInt();
  servo.write(position);
}

// Relay control
void pumpControl(char *command){
  if (strcmp(command, "ON") == 0) {
    digitalWrite(RELAY_PIN, HIGH);

  } else if (strcmp(command, "OFF") == 0) {
    digitalWrite(RELAY_PIN, LOW);

  }
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

void checkWifi(){
  Serial.print("Wifi Status: ");
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  } else{
    Serial.println("Disconnected!");

  }
}

void logAll(){
  Serial.println(F("+-----------------------+"));
  Serial.println(F("| Log all functionality |"));
  Serial.println(F("+-----------------------+"));

  Serial.print(F("Flame state: "));
  Serial.println(flameState);

  Serial.print(F("Temp: "));
  Serial.println(temperature);

  Serial.print(F("Humid: "));
  Serial.println(humidity);
  
  checkWifi();

  Serial.print(F("Blynk: "));
  if(Blynk.connected()){
    Serial.println(F("Connected"));
  } else{
    Serial.println(F("Disconnected"));
  }
}

void setup() {
  Serial.begin(115200);

  // INPUT
  pinMode(FLAME_SENSOR_PIN, INPUT);
  dht.begin();

  // OUTPUT
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(BUILTIN_LED, OUTPUT);

  servo.attach(SERVO_PIN);

  // Connect to blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  checkWifi();

  // Timers for sending data
  timer.setInterval(1000L, sendFlameData); // Flame Sensor every 1 sec
  timer.setInterval(2000L, sendDHTData);   // DHT Data every 2 sec

  timer.setTimeout(5000L, logAll);

}

void loop() {
  Blynk.run();
  timer.run();

  mainLogic(); 
}
