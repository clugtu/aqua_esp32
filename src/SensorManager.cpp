#include "SensorManager.h"

SensorManager::SensorManager() {
  // Initialize sensor data
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    sensorData.temperature[i] = 0.0;
  }
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    sensorData.ph[i] = 0.0;
  }
  sensorData.lastUpdate = 0;
}

void SensorManager::begin() {
  // Initialize multiplexer control pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_EN, OUTPUT);
  
  // Enable multiplexer (active LOW)
  digitalWrite(MUX_EN, LOW);
  
  Serial.println("Testing Multiplexer Control Pins:");
  for (int i = 0; i < 8; i++) {
    selectMuxChannel(i);
    delay(100);
  }
  Serial.println();
}

void SensorManager::selectMuxChannel(int channel) {
  // Set control pins for multiplexer channel selection
  bool s0 = channel & 0x01;
  bool s1 = (channel >> 1) & 0x01;
  bool s2 = (channel >> 2) & 0x01;
  bool s3 = (channel >> 3) & 0x01;
  
  digitalWrite(MUX_S0, s0);
  digitalWrite(MUX_S1, s1);
  digitalWrite(MUX_S2, s2);
  digitalWrite(MUX_S3, s3);
  
  // Debug output for multiplexer switching
  Serial.printf("  [MUX] Channel %d -> S3=%d S2=%d S1=%d S0=%d (Pins: %d,%d,%d,%d)\n", 
                channel, s3, s2, s1, s0, 
                digitalRead(MUX_S3), digitalRead(MUX_S2), digitalRead(MUX_S1), digitalRead(MUX_S0));
  
  delayMicroseconds(10); // Small delay for switching
}

float SensorManager::readTemperature(int sensorIndex) {
  selectMuxChannel(sensorIndex);
  
  // Add delay to ensure multiplexer switching (even without hardware)
  delayMicroseconds(100);
  
  int rawValue = analogRead(TEMP_ADC_PIN);
  
  // Convert to voltage (ESP32 ADC: 0-4095 = 0-3.3V)
  float voltage = (rawValue / 4095.0) * 3.3;
  
  // For testing without sensors, simulate realistic temperature readings
  // Add some variation based on sensor index and time
  float simulatedTemp = 24.0 + (sensorIndex * 0.5) + (sin(millis() / 10000.0) * 2.0);
  
  // Example conversion for TMP36 sensor: (voltage - 0.5) * 100
  // Use simulated data when no real sensors connected
  float temperature = (rawValue < 100) ? simulatedTemp : (voltage - 0.5) * 100.0;
  
  // Debug output
  Serial.printf("    [DEBUG] Temp%d: MUX-CH%d, Raw=%d, Voltage=%.3fV, Result=%.2fC\n", 
                sensorIndex + 1, sensorIndex, rawValue, voltage, temperature);
  
  return temperature;
}

float SensorManager::readPH(int sensorIndex) {
  selectMuxChannel(sensorIndex);
  
  // Add delay to ensure multiplexer switching (even without hardware)
  delayMicroseconds(100);
  
  int rawValue = analogRead(PH_ADC_PIN);
  
  // Convert to voltage (ESP32 ADC: 0-4095 = 0-3.3V)
  float voltage = (rawValue / 4095.0) * 3.3;
  
  // For testing without sensors, simulate realistic pH readings
  // Add some variation based on sensor index and time
  float simulatedPH = 7.0 + (sensorIndex * 0.1) + (cos(millis() / 15000.0) * 0.3);
  
  // Example pH conversion (adjust based on your sensor calibration)
  // Use simulated data when no real sensors connected
  float ph = (rawValue < 100) ? simulatedPH : 7.0 + ((2.5 - voltage) / 0.18);
  
  // Debug output
  Serial.printf("    [DEBUG] pH%d: MUX-CH%d, Raw=%d, Voltage=%.3fV, Result=%.2f\n", 
                sensorIndex + 1, sensorIndex, rawValue, voltage, ph);
  
  return ph;
}

void SensorManager::updateAllReadings() {
  Serial.println("  Reading temperature sensors...");
  // Read all temperature sensors
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    sensorData.temperature[i] = readTemperature(i);
    delay(50); // Small delay between readings for demo purposes
  }
  
  Serial.println();
  Serial.println("  Reading pH sensors...");
  // Read all pH sensors
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    sensorData.ph[i] = readPH(i);
    delay(50); // Small delay between readings for demo purposes
  }
  
  sensorData.lastUpdate = millis();
}

SensorData& SensorManager::getData() {
  return sensorData;
}

void SensorManager::printReadings() {
  // Print temperature summary
  Serial.println("  Temperature Summary:");
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    Serial.printf("    Temp%d: %.2fC\n", i + 1, sensorData.temperature[i]);
  }
  
  Serial.println("  pH Summary:");
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    Serial.printf("    pH%d: %.2f\n", i + 1, sensorData.ph[i]);
  }
}

void SensorManager::printDetailedReadings() {
  Serial.println("Temperature Sensors:");
  updateAllReadings();
  printReadings();
}