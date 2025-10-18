#pragma once
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "SensorController.h"
#include "CalibrationManager.h"
#include "ConfigManager.h"
#include "TemplateManager.h"
#include "Config.h"

class AquaWebServer {
private:
  AsyncWebServer server;
  SensorController* sensorController;
  CalibrationManager* calibrationManager;
  ConfigManager* configManager;
  TemplateManager* templateManager;
  
  void setupRoutes();
  void handleRoot(AsyncWebServerRequest *request);
  void handleApiSensors(AsyncWebServerRequest *request);
  void handleApiTemperature(AsyncWebServerRequest *request);
  void handleApiPH(AsyncWebServerRequest *request);
  void handleApiTDS(AsyncWebServerRequest *request);
  void handleApiStatus(AsyncWebServerRequest *request);
  void handleApiAquariums(AsyncWebServerRequest *request);
  void handleCalibrationPage(AsyncWebServerRequest *request);
  void handleCalibrationStatus(AsyncWebServerRequest *request);
  void handleStartCalibration(AsyncWebServerRequest *request);
  void handleAddCalibrationPoint(AsyncWebServerRequest *request);
  void handleFinalizeCalibration(AsyncWebServerRequest *request);
  void handleCalibrationReading(AsyncWebServerRequest *request);
  void handleHelpPage(AsyncWebServerRequest *request);
  void handleDiagnosticsPage(AsyncWebServerRequest *request);
  void handleAdminPage(AsyncWebServerRequest *request);
  void handleAdminLogin(AsyncWebServerRequest *request);
  void handleConfigPage(AsyncWebServerRequest *request);
  void handleApiConfig(AsyncWebServerRequest *request);
  void handleApiConfigSave(AsyncWebServerRequest *request);
  // Template rendering methods
  String renderDashboard();
  String renderAdminLogin();
  String renderConfig();
  
  // Legacy HTML generation methods (TODO: Convert to templates)
  String generateDashboardHTML();
  String generateCalibrationHTML();
  String generateHelpHTML();
  String generateDiagnosticsHTML();
  String generateAdminHTML();
  String generateConfigHTML();

public:
  AquaWebServer();
  void begin(SensorController* sensors, CalibrationManager* calibration, ConfigManager* config);
  void setSensorController(SensorController* sensors);
  void setCalibrationManager(CalibrationManager* calibration);
  void setConfigManager(ConfigManager* config);
};