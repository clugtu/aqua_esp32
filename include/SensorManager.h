#pragma once
#include <Arduino.h>
#include "Config.h"

struct SensorData {
  float temperature[NUM_TEMP_SENSORS];
  float ph[NUM_PH_SENSORS];
  unsigned long lastUpdate;
};

class SensorManager {
private:
  SensorData sensorData;
  
  void selectMuxChannel(int channel);
  float readTemperature(int sensorIndex);
  float readPH(int sensorIndex);

public:
  SensorManager();
  void begin();
  void updateAllReadings();
  SensorData& getData();
  void printReadings();
  void printDetailedReadings();
};