#include "SensorController.h"

SensorController::SensorController() 
  : tempSensors(&mux, TEMP_ADC_PIN), phSensors(&mux, PH_ADC_PIN), tdsSensors(&mux, TDS_ADC_PIN) {}

void SensorController::begin() {
  Serial.println("Sensor Controller Initializing...");
  
  // Initialize multiplexer first
  mux.begin();
  
  // Initialize sensor classes
  tempSensors.begin();
  phSensors.begin();
  tdsSensors.begin();
  
  Serial.println("All sensor systems ready");
  Serial.println();
}

void SensorController::updateAllReadings() {
  tempSensors.updateAllReadings();
  Serial.println();
  phSensors.updateAllReadings();
  Serial.println();
  tdsSensors.updateAllReadings();
}

void SensorController::printAllReadings() {
  tempSensors.printReadings();
  Serial.println();
  phSensors.printReadings();
  Serial.println();
  tdsSensors.printReadings();
}

void SensorController::printDetailedReadings() {
  tempSensors.printDetailedReadings();
  Serial.println();
  phSensors.printDetailedReadings();
  Serial.println();
  tdsSensors.printDetailedReadings();
}

TemperatureSensor& SensorController::getTemperatureSensors() {
  return tempSensors;
}

PHSensor& SensorController::getPHSensors() {
  return phSensors;
}

TDSSensor& SensorController::getTDSSensors() {
  return tdsSensors;
}

MultiplexerController& SensorController::getMultiplexer() {
  return mux;
}