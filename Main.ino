#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define DHT11_PIN  23 
DHT dht11(DHT11_PIN, DHT11);
Adafruit_BMP085 bmp;
float calibration_factor = 40.0;

void setup() {
  Serial.begin(115200); 
  delay(2000);
  
  Serial.println("Initializing sensors...");
  

  if (!bmp.begin()) {
    Serial.println("BMP085 not found. Check wiring!");
  } else {
    Serial.println("BMP085 initialized successfully");
  }
  
  dht11.begin();
  Serial.println("DHT11 initialized");
}

void loop() {
  BMPRead();
  ReadTemperature();
  READMQ135();
  
  delay(5000); 
}

void READMQ135() {
   float temp = dht11.readTemperature();
  float humidity = dht11.readHumidity();
  float voltage = analogRead(35) * (3.3 / 4095.0);
  
  float corrected_voltage = voltage * (1.0 + (25.0 - temp) * 0.02) * (1.0 + (50.0 - humidity) * 0.0015);
  
  float ppm = 116.6020682 * pow((3.3 - corrected_voltage)/corrected_voltage, -2.769034857) * calibration_factor;
  
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print("V, PPM: ");
  Serial.print(ppm, 1);
  Serial.print(", Cal Factor: ");
  Serial.println(calibration_factor, 2);
  
  delay(2000);
}

void BMPRead() {
  if (!bmp.begin()) {
    return; 
  }
  
  Serial.println("=== BMP085 Readings ===");
  
  float tempBMP = bmp.readTemperature();
  if (!isnan(tempBMP)) {
    Serial.print("BMP Temperature = ");
    Serial.print(tempBMP);
    Serial.println(" *C");
  } else {
    Serial.println("Failed to read BMP temperature");
  }
  
  long pressure = bmp.readPressure();
  if (pressure != 0) {
    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" Pa");
  }
  
  Serial.println();
}

void ReadTemperature() {
  Serial.println("=== DHT11 Readings ===");
  
  float humi = dht11.readHumidity();
  float tempC = dht11.readTemperature();
  
  if (isnan(tempC) || isnan(humi)) {
    Serial.println("Failed to read from DHT11 sensor!");
    Serial.println("DHT11 wiring: VCC->3.3V, GND->GND, DATA->GPIO17");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");
    Serial.print("  |  Temperature: ");
    Serial.print(tempC);
    Serial.println("°C");
  }
  Serial.println();
}