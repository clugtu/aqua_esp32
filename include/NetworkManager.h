#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "ConfigManager.h"

class NetworkManager {
private:
  bool isConnected;
  ConfigManager* configMgr;
  
public:
  NetworkManager();
  void begin(ConfigManager* config);
  bool connect();
  void printConnectionDetails();
  bool checkConnection();
  String getIP();
  int getRSSI();
};