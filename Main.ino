#include <DHT.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_BMP085.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define DHT11_PIN 23 
#define SensorUID "GHJKOIUHN>:PMKL<MJ21447622"

DHT dht11(DHT11_PIN, DHT11);
Adafruit_BMP085 bmp;

float calibration_factor = 40.0;

const char* ssids[] = {
  "LTE-WiFi_D916", 
  "Dungeon Master"
};

const char* passwords[] = {
  "db6ed916",
  "00000000"
};

const int networksCount = 2;

const char* serverURL = "http://195.62.49.9:5000/api/v1/temp";

// Данные для спячки
// Время сна в микросекундах (5 минут)
#define uS_TO_S_FACTOR 1000000
#define SLEEP_TIME 5 * 60 * uS_TO_S_FACTOR

RTC_DATA_ATTR int lastConnectedNetwork = -1;

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(2000);
  Serial.begin(115200); 
  delay(2000);
  
  Serial.println("ESP32 active...");

  bmp.begin();
  dht11.begin();
  
  delay(5000);

  read_sensor_and_send_data();

  Serial.println("Going to deep sleep for 5 minutes...");
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  digitalWrite(2, LOW);
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  
  esp_deep_sleep_start();
}

void loop() {}

void read_sensor_and_send_data() {
  if (connectToAnyWiFi()) {
    Serial.println("\nConnected to WiFi!");
    
   /* float dht_temperature = dht_read_temperature();
    float dht_humidity = dht_read_humidity();
    float bmp_temperature = bmp_read_temperature();
    float bmp_pressure = bmp_read_pressure();
    float mq_analog = mq_read_analog();
    float mq_ppm = mq_read_gas_ppm();*/
    
   // sendDataToServer(dht_temperature, dht_humidity, bmp_pressure, bmp_temperature, mq_ppm, mq_analog);  
   sendDataToServer(27.0, 50, 101560, 26.8, 440, 1900);
  } else {
    Serial.println("\nFailed to connect to WiFi!");
  }
}

bool connectToAnyWiFi() {
  if (lastConnectedNetwork >= 0 && lastConnectedNetwork < networksCount) {
    Serial.print("Trying last connected network: ");
    Serial.println(ssids[lastConnectedNetwork]);
    
    if (connectToWiFi(ssids[lastConnectedNetwork], passwords[lastConnectedNetwork], 8)) {
      return true;
    }
  }

  for (int i = 0; i < networksCount; i++) {
    if (i == lastConnectedNetwork) continue;
    
    Serial.print("Trying network ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(ssids[i]);
    
    if (connectToWiFi(ssids[i], passwords[i], 10)) {
      lastConnectedNetwork = i;
      return true;
    }
  }
  
  return false;
}

bool connectToWiFi(const char* ssid, const char* password, int maxAttempts) {
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected to: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    return true;
  }
  
  Serial.println("\nFailed to connect");
  WiFi.disconnect();
  delay(100);
  return false;
}

void sendDataToServer(float dht_temp, float dht_hum, long bmp_pressure, float bmp_temp, float mq_ppm, float mq_bad_data) {
  HTTPClient http;
  
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  
  JsonDocument doc;
  doc["sensor_uid"] = SensorUID;
  doc["dht_temperature"] = dht_temp;
  doc["dht_humidity"] = dht_hum;
  doc["bmp_pressure"] = bmp_pressure;
  doc["bmp_temperature"] = bmp_temp;
  doc["mq_ppm"] = mq_ppm;
  doc["mq_bad_data"] = mq_ppm;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.print("Sending JSON: ");
  Serial.println(jsonString);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

// Получение газа в ppm
float mq_read_gas_ppm() {
  float temp = dht11.readTemperature();
  float humidity = dht11.readHumidity();
  float voltage = analogRead(35) * (3.3 / 4095.0);
  
  float corrected_voltage = voltage * (1.0 + (25.0 - temp) * 0.02) * (1.0 + (50.0 - humidity) * 0.0015);
  
  float ppm = 116.6020682 * pow((3.3 - corrected_voltage)/corrected_voltage, -2.769034857) * calibration_factor;

  return ppm;
}

// Получение аналогового значения газа
float mq_read_analog() {
  return analogRead(35);
}

// В Цельсиях
float bmp_read_temperature() {
  return bmp.readTemperature();
}

// В Паскалях
float bmp_read_pressure() {
  return bmp.readPressure();
}

// В Цельсиях
float dht_read_temperature() {
  return dht11.readTemperature();
}

// В процентах
float dht_read_humidity() {
  return dht11.readHumidity();
}