#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

class ConfigManager {
private:
  JsonDocument config;
  bool configLoaded;
  
  bool initSPIFFS();
  bool loadConfigFile();
  
public:
  ConfigManager();
  bool begin();
  
  // WiFi configuration
  String getWifiSSID();
  String getWifiPassword();
  
  // System configuration
  String getDeviceName();
  int getSerialBaud();
  int getSensorReadInterval();
  int getPrintInterval();
  
  // NO ICONS policy configuration
  bool getUseIcons();
  bool getUseEmoji();
  bool getAsciiOnly();
  String getOutputPolicy();
  
  // Sensor configuration
  int getTemperatureCount();
  int getPHCount();
  int getTDSCount();
  
  // Hardware configuration
  int getLedPin();
  int getTempAdcPin();
  int getPHAdcPin();
  int getTDSAdcPin();
  int getMuxS0();
  int getMuxS1();
  int getMuxS2();
  int getMuxS3();
  int getMuxEnable();
  
  // Utility methods
  bool isLoaded();
  void printConfig();
};