#pragma once
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "SensorController.h"
#include "CalibrationManager.h"
#include "Config.h"

class AquaWebServer {
private:
  AsyncWebServer server;
  SensorController* sensorController;
  CalibrationManager* calibrationManager;
  
  void setupRoutes();
  void handleRoot(AsyncWebServerRequest *request);
  void handleApiSensors(AsyncWebServerRequest *request);
  void handleApiTemperature(AsyncWebServerRequest *request);
  void handleApiPH(AsyncWebServerRequest *request);
  void handleApiTDS(AsyncWebServerRequest *request);
  void handleApiStatus(AsyncWebServerRequest *request);
  void handleCalibrationPage(AsyncWebServerRequest *request);
  void handleCalibrationStatus(AsyncWebServerRequest *request);
  void handleStartCalibration(AsyncWebServerRequest *request);
  void handleAddCalibrationPoint(AsyncWebServerRequest *request);
  void handleFinalizeCalibration(AsyncWebServerRequest *request);
  void handleCalibrationReading(AsyncWebServerRequest *request);
  void handleHelpPage(AsyncWebServerRequest *request);
  void handleDiagnosticsPage(AsyncWebServerRequest *request);
  String generateDashboardHTML();
  String generateCalibrationHTML();
  String generateHelpHTML();
  String generateDiagnosticsHTML();

public:
  AquaWebServer();
  void begin(SensorController* sensors, CalibrationManager* calibration);
  void setSensorController(SensorController* sensors);
  void setCalibrationManager(CalibrationManager* calibration);
};