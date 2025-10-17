#pragma once
#include <Arduino.h>
#include "Config.h"
#include "MultiplexerController.h"

struct TemperatureData {
  float readings[NUM_TEMP_SENSORS];
  unsigned long lastUpdate;
};

class TemperatureSensor {
private:
  MultiplexerController* mux;
  TemperatureData data;
  int adcPin;
  
  float readSingleSensor(int sensorIndex);
  float convertVoltageToTemperature(float voltage, int sensorIndex);

public:
  TemperatureSensor(MultiplexerController* multiplexer, int pin);
  void begin();
  void updateAllReadings();
  TemperatureData& getData();
  float getReading(int sensorIndex);
  void printReadings();
  void printDetailedReadings();
  int getSensorCount() const;
};