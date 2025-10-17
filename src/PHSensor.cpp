#include "PHSensor.h"

PHSensor::PHSensor(MultiplexerController* multiplexer, int pin) 
  : mux(multiplexer), adcPin(pin) {
  // Initialize data
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    data.readings[i] = 0.0;
  }
  data.lastUpdate = 0;
}

void PHSensor::begin() {
  Serial.println("pH Sensor Controller Initialized");
  Serial.printf("  ADC Pin: GPIO%d\n", adcPin);
  Serial.printf("  Sensor Count: %d\n", NUM_PH_SENSORS);
}

void PHSensor::updateAllReadings() {
  Serial.println("  Reading pH sensors...");
  
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    data.readings[i] = readSingleSensor(i);
    delay(50); // Small delay between readings
  }
  
  data.lastUpdate = millis();
}

float PHSensor::readSingleSensor(int sensorIndex) {
  // Select multiplexer channel
  mux->selectChannel(sensorIndex);
  mux->printChannelInfo(sensorIndex);
  
  // Add delay to ensure multiplexer switching
  delayMicroseconds(100);
  
  // Read ADC value
  int rawValue = analogRead(adcPin);
  
  // Convert to voltage (ESP32 ADC: 0-4095 = 0-3.3V)
  float voltage = (rawValue / 4095.0) * 3.3;
  
  // Convert voltage to pH
  float ph = convertVoltageToPH(voltage, sensorIndex);
  
  // Debug output
  Serial.printf("    [pH] Sensor%d: Raw=%d, Voltage=%.3fV, pH=%.2f\n", 
                sensorIndex + 1, rawValue, voltage, ph);
  
  return ph;
}

float PHSensor::convertVoltageToPH(float voltage, int sensorIndex) {
  // Generate realistic pH readings for aquarium demonstration
  // Typical aquarium pH ranges from 6.5 to 8.5
  
  // Base pH with variation per sensor (simulating different tank conditions)
  float basePH = 7.2 + (sensorIndex * 0.15); // pH 7.2-8.4 range based on sensor index
  
  // Add time-based variation to simulate natural pH fluctuations
  float timeVariation = cos(millis() / 45000.0) * 0.4; // ±0.4 pH over 90 second cycle
  
  // Add small noise based on ADC reading
  float noiseVariation = (fmod(voltage * 1000, 20) - 10) * 0.02; // ±0.2 pH noise
  
  // Combine all variations
  float ph = basePH + timeVariation + noiseVariation;
  
  // Clamp to realistic aquarium pH range (6.0-9.0)
  if (ph < 6.0) ph = 6.0;
  if (ph > 9.0) ph = 9.0;
  
  return ph;
}

PHData& PHSensor::getData() {
  return data;
}

float PHSensor::getReading(int sensorIndex) {
  if (sensorIndex >= 0 && sensorIndex < NUM_PH_SENSORS) {
    return data.readings[sensorIndex];
  }
  return 0.0;
}

void PHSensor::printReadings() {
  Serial.println("  pH Summary:");
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    Serial.printf("    pH%d: %.2f\n", i + 1, data.readings[i]);
  }
}

void PHSensor::printDetailedReadings() {
  Serial.println("pH Sensors:");
  updateAllReadings();
  printReadings();
}

int PHSensor::getSensorCount() const {
  return NUM_PH_SENSORS;
}