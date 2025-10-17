#pragma once

#include <Arduino.h>
#include "MultiplexerController.h"

// TDS sensor constants
#define NUM_TDS_SENSORS 8
#define TDS_ADC_PIN 35        // ADC1_CH7 - TDS sensors multiplexer
#define TDS_VREF 3.3          // Reference voltage
#define TDS_SCOUNT 30         // Number of samples for averaging
#define TDS_KVALUE 1.0        // K value for TDS calculation

// Structure to hold TDS sensor data
struct TDSData {
  float readings[NUM_TDS_SENSORS];
  unsigned long lastUpdate;
};

class TDSSensor {
private:
  MultiplexerController* mux;
  int adcPin;
  TDSData data;
  
  // Calibration values
  float kValue = TDS_KVALUE;
  float temperature = 25.0;  // Default temperature for compensation
  
  // Moving average buffer
  int analogBuffer[TDS_SCOUNT];
  int analogBufferTemp[TDS_SCOUNT];
  int analogBufferIndex = 0;
  
  // Private methods
  float convertToTDS(int rawValue);
  float convertToEC(float tds) const;
  int getMedianNum(int bArray[], int iFilterLen);
  float calculateTDSValue(int rawValue, float temperature);

public:
  TDSSensor(MultiplexerController* multiplexer, int pin);
  
  void begin();
  void updateAllReadings();
  float readSingleSensor(int sensorIndex);
  
  // Getters
  TDSData getData() const { return data; }
  int getSensorCount() const;
  float getTDSReading(int index) const;
  float getECReading(int index) const;
  
  // Setters
  void setKValue(float k) { kValue = k; }
  void setTemperature(float temp) { temperature = temp; }
  
  // Display methods
  void printReadings();
  void printDetailedReadings();
};