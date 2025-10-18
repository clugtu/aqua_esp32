#pragma once
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <ArduinoJson.h>
#include "SensorController.h"
#include "TemplateManager.h"

class SecureWebServer {
private:
  WiFiServer* server;
  WiFiServerSecure* secureServer;
  SensorController* sensorController;
  TemplateManager* templateManager;
  
  bool sslEnabled;
  String certificatePEM;
  String privateKeyPEM;
  
  void handleClient(WiFiClient client, bool isSecure);
  void sendResponse(WiFiClient& client, int code, const String& contentType, const String& content, bool isSecure);
  void addSecurityHeaders(WiFiClient& client);
  String parseRequest(WiFiClient& client, String& method, String& path);
  
public:
  SecureWebServer(int httpPort = 80, int httpsPort = 443);
  ~SecureWebServer();
  
  bool begin(SensorController* sensors);
  bool loadSSLCertificates();
  void handleClients();
  
  // Route handlers
  void handleRoot(WiFiClient& client, bool isSecure);
  void handleAPI(WiFiClient& client, const String& path, bool isSecure);
};