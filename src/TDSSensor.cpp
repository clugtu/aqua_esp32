#include "TDSSensor.h"

TDSSensor::TDSSensor(MultiplexerController* multiplexer, int pin) 
  : mux(multiplexer), adcPin(pin) {
  // Initialize data structure
  data.lastUpdate = 0;
  for (int i = 0; i < NUM_TDS_SENSORS; i++) {
    data.readings[i] = 0.0;
  }
  
  // Initialize analog buffer
  for (int i = 0; i < TDS_SCOUNT; i++) {
    analogBuffer[i] = 0;
    analogBufferTemp[i] = 0;
  }
}

void TDSSensor::begin() {
  Serial.println("TDS Sensor Controller Initialized");
  Serial.printf("  ADC Pin: GPIO%d\n", adcPin);
  Serial.printf("  Sensor Count: %d\n", NUM_TDS_SENSORS);
  Serial.printf("  K Value: %.2f\n", kValue);
}

void TDSSensor::updateAllReadings() {
  Serial.println("  Reading TDS sensors...");
  
  for (int i = 0; i < NUM_TDS_SENSORS; i++) {
    data.readings[i] = readSingleSensor(i);
    delay(50); // Small delay between readings
  }
  
  data.lastUpdate = millis();
}

float TDSSensor::readSingleSensor(int sensorIndex) {
  // Select multiplexer channel
  mux->selectChannel(sensorIndex);
  delay(10); // Allow settling time
  
  // Take multiple readings for better accuracy
  int sum = 0;
  const int numReadings = 10;
  
  for (int i = 0; i < numReadings; i++) {
    sum += analogRead(adcPin);
    delay(2);
  }
  
  int rawValue = sum / numReadings;
  
  // Convert to voltage
  float voltage = (rawValue * TDS_VREF) / 4095.0;
  
  // Calculate TDS value with temperature compensation
  float tdsValue = calculateTDSValue(rawValue, temperature);
  
  // Debug output
  Serial.printf("    [TDS] Sensor%d: Raw=%d, Voltage=%.3fV, TDS=%.2f ppm\n", 
                sensorIndex + 1, rawValue, voltage, tdsValue);
  
  return tdsValue;
}

float TDSSensor::calculateTDSValue(int rawValue, float temperature) {
  // Convert raw ADC value to voltage
  float averageVoltage = (rawValue * TDS_VREF) / 4095.0;
  
  // Temperature compensation coefficient
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  
  // Compensate voltage based on temperature
  float compensationVoltage = averageVoltage / compensationCoefficient;
  
  // Convert voltage to TDS value using the k value
  // TDS formula: TDS = (133.42 * voltage^3 - 255.86 * voltage^2 + 857.39 * voltage) * kValue
  float tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 
                   255.86 * compensationVoltage * compensationVoltage + 
                   857.39 * compensationVoltage) * kValue;
  
  // Ensure TDS value is not negative
  if (tdsValue < 0) {
    tdsValue = 0;
  }
  
  return tdsValue;
}

float TDSSensor::convertToTDS(int rawValue) {
  return calculateTDSValue(rawValue, temperature);
}

float TDSSensor::convertToEC(float tds) const {
  // Convert TDS (ppm) to EC (uS/cm)
  // Typical conversion: EC (uS/cm) = TDS (ppm) * 2
  return tds * 2.0;
}

int TDSSensor::getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
    
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    
  return bTemp;
}

int TDSSensor::getSensorCount() const {
  return NUM_TDS_SENSORS;
}

float TDSSensor::getTDSReading(int index) const {
  if (index >= 0 && index < NUM_TDS_SENSORS) {
    return data.readings[index];
  }
  return 0.0;
}

float TDSSensor::getECReading(int index) const {
  if (index >= 0 && index < NUM_TDS_SENSORS) {
    return convertToEC(data.readings[index]);
  }
  return 0.0;
}

void TDSSensor::printReadings() {
  Serial.println("  TDS Summary:");
  for (int i = 0; i < NUM_TDS_SENSORS; i++) {
    float ec = convertToEC(data.readings[i]);
    Serial.printf("    TDS%d: %.2f ppm / %.2f uS/cm\n", 
                  i + 1, data.readings[i], ec);
  }
}

void TDSSensor::printDetailedReadings() {
  Serial.println("TDS Sensors:");
  updateAllReadings();
  printReadings();
}