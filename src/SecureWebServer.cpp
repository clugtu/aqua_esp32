#include "SecureWebServer.h"
#include "SPIFFS.h"

SecureWebServer::SecureWebServer(int httpPort, int httpsPort) {
  server = new WiFiServer(httpPort);
  secureServer = new WiFiServerSecure(httpsPort);
  sensorController = nullptr;
  templateManager = nullptr;
  sslEnabled = false;
}

SecureWebServer::~SecureWebServer() {
  delete server;
  delete secureServer;
  delete templateManager;
}

bool SecureWebServer::loadSSLCertificates() {
  if (!SPIFFS.exists("/ssl/cert.pem") || !SPIFFS.exists("/ssl/key.pem")) {
    Serial.println("SSL certificate files not found");
    return false;
  }
  
  File certFile = SPIFFS.open("/ssl/cert.pem", "r");
  if (!certFile) {
    Serial.println("Failed to open certificate file");
    return false;
  }
  certificatePEM = certFile.readString();
  certFile.close();
  
  File keyFile = SPIFFS.open("/ssl/key.pem", "r");
  if (!keyFile) {
    Serial.println("Failed to open private key file");
    return false;
  }
  privateKeyPEM = keyFile.readString();
  keyFile.close();
  
  // Set SSL certificates
  secureServer->loadCertChain(certificatePEM.c_str(), certificatePEM.length());
  secureServer->loadPrivateKey(privateKeyPEM.c_str(), privateKeyPEM.length());
  
  Serial.println("SSL certificates loaded successfully");
  sslEnabled = true;
  return true;
}

bool SecureWebServer::begin(SensorController* sensors) {
  sensorController = sensors;
  templateManager = new TemplateManager(true);
  
  // Start HTTP server
  server->begin();
  Serial.println("HTTP server started on port 80");
  
  // Try to start HTTPS server
  if (loadSSLCertificates()) {
    secureServer->begin();
    Serial.println("HTTPS server started on port 443");
    Serial.println("Real HTTPS is now available!");
  } else {
    Serial.println("HTTPS server failed to start - using HTTP only");
  }
  
  Serial.println();
  Serial.println("Secure Web Server Started!");
  Serial.println("+---------------------------------------+");
  Serial.printf("| HTTP:  http://%-22s |\n", WiFi.localIP().toString().c_str());
  if (sslEnabled) {
    Serial.printf("| HTTPS: https://%-21s |\n", WiFi.localIP().toString().c_str());
  }
  Serial.println("+---------------------------------------+");
  
  return true;
}

void SecureWebServer::handleClients() {
  // Handle HTTP clients
  WiFiClient httpClient = server->available();
  if (httpClient) {
    handleClient(httpClient, false);
  }
  
  // Handle HTTPS clients (if SSL is enabled)
  if (sslEnabled) {
    WiFiClientSecure httpsClient = secureServer->available();
    if (httpsClient) {
      handleClient(httpsClient, true);
    }
  }
}

void SecureWebServer::handleClient(WiFiClient client, bool isSecure) {
  if (!client) return;
  
  String currentLine = "";
  String method = "";
  String path = "";
  
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n') {
        if (currentLine.length() == 0) {
          // End of headers - process request
          if (path == "/" || path == "") {
            handleRoot(client, isSecure);
          } else if (path.startsWith("/api/")) {
            handleAPI(client, path, isSecure);
          } else {
            sendResponse(client, 404, "text/plain", "Not Found", isSecure);
          }
          break;
        } else {
          // Parse first line for method and path
          if (method == "" && currentLine.startsWith("GET ")) {
            method = "GET";
            int spaceIndex = currentLine.indexOf(' ');
            int secondSpaceIndex = currentLine.indexOf(' ', spaceIndex + 1);
            if (spaceIndex != -1 && secondSpaceIndex != -1) {
              path = currentLine.substring(spaceIndex + 1, secondSpaceIndex);
            }
          }
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }
  
  client.stop();
}

void SecureWebServer::sendResponse(WiFiClient& client, int code, const String& contentType, const String& content, bool isSecure) {
  client.println("HTTP/1.1 " + String(code) + " OK");
  client.println("Content-Type: " + contentType);
  client.println("Content-Length: " + String(content.length()));
  
  // Add security headers
  addSecurityHeaders(client);
  
  client.println(); // End headers
  client.print(content);
}

void SecureWebServer::addSecurityHeaders(WiFiClient& client) {
  client.println("X-Content-Type-Options: nosniff");
  client.println("X-Frame-Options: SAMEORIGIN");
  client.println("X-XSS-Protection: 1; mode=block");
  client.println("Referrer-Policy: strict-origin-when-cross-origin");
  client.println("Content-Security-Policy: default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
}

void SecureWebServer::handleRoot(WiFiClient& client, bool isSecure) {
  if (!templateManager) {
    sendResponse(client, 500, "text/plain", "Template manager not initialized", isSecure);
    return;
  }
  
  String html = templateManager->loadTemplate("dashboard.html");
  if (html.length() == 0) {
    sendResponse(client, 500, "text/plain", "Failed to load dashboard template", isSecure);
    return;
  }
  
  // Replace template variables
  html.replace("{{DEVICE_NAME}}", "Aquarium Monitor");
  html.replace("{{VERSION}}", "2.0.0");
  
  sendResponse(client, 200, "text/html", html, isSecure);
}

void SecureWebServer::handleAPI(WiFiClient& client, const String& path, bool isSecure) {
  if (!sensorController) {
    sendResponse(client, 500, "application/json", "{\"error\":\"Sensor controller not initialized\"}", isSecure);
    return;
  }
  
  JsonDocument doc;
  
  if (path == "/api/sensors") {
    // Return all sensor data
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    
    // Add temperature data
    JsonArray tempArray = sensors["temperature"].to<JsonArray>();
    for (int i = 0; i < 8; i++) {
      tempArray.add(sensorController->getTemperature(i));
    }
    
    // Add pH data  
    JsonArray phArray = sensors["ph"].to<JsonArray>();
    for (int i = 0; i < 8; i++) {
      phArray.add(sensorController->getPH(i));
    }
    
    // Add TDS data
    JsonArray tdsArray = sensors["tds"].to<JsonArray>();
    for (int i = 0; i < 8; i++) {
      tdsArray.add(sensorController->getTDS(i));
    }
    
  } else if (path == "/api/status") {
    doc["status"] = "online";
    doc["uptime"] = millis();
    doc["secure"] = isSecure;
    doc["ssl_enabled"] = sslEnabled;
    
  } else {
    sendResponse(client, 404, "application/json", "{\"error\":\"API endpoint not found\"}", isSecure);
    return;
  }
  
  String response;
  serializeJson(doc, response);
  sendResponse(client, 200, "application/json", response, isSecure);
}