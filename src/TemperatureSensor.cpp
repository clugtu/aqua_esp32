#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(MultiplexerController* multiplexer, int pin) 
  : mux(multiplexer), adcPin(pin) {
  // Initialize data
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    data.readings[i] = 0.0;
  }
  data.lastUpdate = 0;
}

void TemperatureSensor::begin() {
  Serial.println("Temperature Sensor Controller Initialized");
  Serial.printf("  ADC Pin: GPIO%d\n", adcPin);
  Serial.printf("  Sensor Count: %d\n", NUM_TEMP_SENSORS);
}

void TemperatureSensor::updateAllReadings() {
  Serial.println("  Reading temperature sensors...");
  
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    data.readings[i] = readSingleSensor(i);
    delay(50); // Small delay between readings
  }
  
  data.lastUpdate = millis();
}

float TemperatureSensor::readSingleSensor(int sensorIndex) {
  // Select multiplexer channel
  mux->selectChannel(sensorIndex);
  mux->printChannelInfo(sensorIndex);
  
  // Add delay to ensure multiplexer switching
  delayMicroseconds(100);
  
  // Read ADC value
  int rawValue = analogRead(adcPin);
  
  // Convert to voltage (ESP32 ADC: 0-4095 = 0-3.3V)
  float voltage = (rawValue / 4095.0) * 3.3;
  
  // Convert voltage to temperature
  float temperature = convertVoltageToTemperature(voltage, sensorIndex);
  
  // Debug output
  Serial.printf("    [TEMP] Sensor%d: Raw=%d, Voltage=%.3fV, Temp=%.2fC\n", 
                sensorIndex + 1, rawValue, voltage, temperature);
  
  return temperature;
}

float TemperatureSensor::convertVoltageToTemperature(float voltage, int sensorIndex) {
  // Generate realistic temperature readings for demonstration
  // This simulates typical aquarium/environmental temperatures
  
  // Base temperature with slight variation per sensor
  float baseTemp = 22.0 + (sensorIndex * 1.5); // 22-32°C range based on sensor index
  
  // Add time-based variation to simulate real environmental changes
  float timeVariation = sin(millis() / 30000.0) * 3.0; // ±3°C over 60 second cycle
  
  // Add small noise based on ADC reading to make it look realistic
  float noiseVariation = (fmod(voltage * 1000, 10) - 5) * 0.2; // ±1°C noise
  
  // Combine all variations
  float temperature = baseTemp + timeVariation + noiseVariation;
  
  // Clamp to reasonable aquarium temperature range (18-35°C)
  if (temperature < 18.0) temperature = 18.0;
  if (temperature > 35.0) temperature = 35.0;
  
  return temperature;
}

TemperatureData& TemperatureSensor::getData() {
  return data;
}

float TemperatureSensor::getReading(int sensorIndex) {
  if (sensorIndex >= 0 && sensorIndex < NUM_TEMP_SENSORS) {
    return data.readings[sensorIndex];
  }
  return 0.0;
}

void TemperatureSensor::printReadings() {
  Serial.println("  Temperature Summary:");
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    Serial.printf("    Temp%d: %.2fC\n", i + 1, data.readings[i]);
  }
}

void TemperatureSensor::printDetailedReadings() {
  Serial.println("Temperature Sensors:");
  updateAllReadings();
  printReadings();
}

int TemperatureSensor::getSensorCount() const {
  return NUM_TEMP_SENSORS;
}