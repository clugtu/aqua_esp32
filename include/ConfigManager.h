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
  
  // Security configuration
  String getAdminUsername();
  String getAdminPassword();
  bool isSSLEnabled();
  int getHTTPSPort();
  int getHTTPPort();
  
  // Aquarium management
  int getAquariumCount();
  String getAquariumName(int index);
  String getAquariumID(int index);
  String getAquariumDescription(int index);
  bool isAquariumEnabled(int index);
  
  // Sensor ranges per aquarium
  float getTemperatureMin(int aquariumIndex);
  float getTemperatureMax(int aquariumIndex);
  float getPHMin(int aquariumIndex);
  float getPHMax(int aquariumIndex);
  float getTDSMin(int aquariumIndex);
  float getTDSMax(int aquariumIndex);
  
  // Sensor assignments per aquarium
  int getTemperatureSensorCount(int aquariumIndex);
  int getTemperatureSensorID(int aquariumIndex, int sensorIndex);
  int getPHSensorCount(int aquariumIndex);
  int getPHSensorID(int aquariumIndex, int sensorIndex);
  int getTDSSensorCount(int aquariumIndex);
  int getTDSSensorID(int aquariumIndex, int sensorIndex);
  
  // Range checking utilities
  bool isTemperatureInRange(int aquariumIndex, float value);
  bool isPHInRange(int aquariumIndex, float value);
  bool isTDSInRange(int aquariumIndex, float value);
  
  // Utility methods
  bool isLoaded();
  void printConfig();
};