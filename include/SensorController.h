#pragma once
#include <Arduino.h>
#include "MultiplexerController.h"
#include "TemperatureSensor.h"
#include "PHSensor.h"
#include "TDSSensor.h"

class SensorController {
private:
  MultiplexerController mux;
  TemperatureSensor tempSensors;
  PHSensor phSensors;
  TDSSensor tdsSensors;

public:
  SensorController();
  void begin();
  void updateAllReadings();
  void printAllReadings();
  void printDetailedReadings();
  
  // Accessor methods
  TemperatureSensor& getTemperatureSensors();
  PHSensor& getPHSensors();
  TDSSensor& getTDSSensors();
  MultiplexerController& getMultiplexer();
};