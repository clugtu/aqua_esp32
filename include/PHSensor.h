#pragma once
#include <Arduino.h>
#include "Config.h"
#include "MultiplexerController.h"

struct PHData {
  float readings[NUM_PH_SENSORS];
  unsigned long lastUpdate;
};

class PHSensor {
private:
  MultiplexerController* mux;
  PHData data;
  int adcPin;
  
  float readSingleSensor(int sensorIndex);
  float convertVoltageToPH(float voltage, int sensorIndex);

public:
  PHSensor(MultiplexerController* multiplexer, int pin);
  void begin();
  void updateAllReadings();
  PHData& getData();
  float getReading(int sensorIndex);
  void printReadings();
  void printDetailedReadings();
  int getSensorCount() const;
};