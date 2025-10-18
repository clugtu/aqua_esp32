#include "NetworkManager.h"

NetworkManager::NetworkManager() : isConnected(false), configMgr(nullptr) {}

void NetworkManager::begin(ConfigManager* config) {
  configMgr = config;
  Serial.println("Initializing Network Manager...");
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
}

bool NetworkManager::connect() {
  if (!configMgr) {
    Serial.println("Error: NetworkManager: No configuration manager available");
    return false;
  }
  
  String ssid = configMgr->getWifiSSID();
  String password = configMgr->getWifiPassword();
  
  Serial.println("DEBUG: WiFi Configuration from ConfigManager:");
  Serial.printf("DEBUG: SSID: '%s'\n", ssid.c_str());
  Serial.printf("DEBUG: Password: '%s'\n", password.c_str());
  Serial.println("Connecting to WiFi...");
  Serial.printf("SSID: %s\n", ssid.c_str());
  Serial.print("Status: ");
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  IPAddress lastIP = IPAddress(0, 0, 0, 0);
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    
    // Check if we got an IP address during connection process
    IPAddress currentIP = WiFi.localIP();
    if (currentIP != lastIP && currentIP != IPAddress(0, 0, 0, 0)) {
      Serial.println();
      Serial.printf("DHCP Lease Obtained: %s\n", currentIP.toString().c_str());
      lastIP = currentIP;
    }
    
    // Show detailed connection status every 5 attempts
    if (attempts % 5 == 4) {
      Serial.println();
      Serial.printf("   Connection attempt %d/20\n", attempts + 1);
      Serial.printf("   WiFi Status: %d\n", WiFi.status());
      if (WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
        Serial.printf("   Current IP: %s\n", WiFi.localIP().toString().c_str());
      }
      Serial.print("   Continuing: ");
    }
    
    attempts++;
  }
  
  Serial.println();
  isConnected = (WiFi.status() == WL_CONNECTED);
  
  if (isConnected) {
    printConnectionDetails();
  } else {
    Serial.println("WiFi Connection Failed!");
    Serial.println("Continuing in offline mode...");
  }
  
  return isConnected;
}

void NetworkManager::printConnectionDetails() {
  Serial.println("WiFi Connected Successfully!");
  Serial.println("+--- DHCP Network Configuration ----+");
  Serial.printf("| IP Address: %-18s |\n", WiFi.localIP().toString().c_str());
  Serial.printf("| Gateway:    %-18s |\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("| DNS Server: %-18s |\n", WiFi.dnsIP().toString().c_str());
  Serial.printf("| Subnet:     %-18s |\n", WiFi.subnetMask().toString().c_str());
  Serial.printf("| RSSI:       %-15d dBm |\n", WiFi.RSSI());
  Serial.printf("| Encryption: %-18s |\n", 
                (WiFi.encryptionType(0) == WIFI_AUTH_OPEN) ? "Open" :
                (WiFi.encryptionType(0) == WIFI_AUTH_WEP) ? "WEP" :
                (WiFi.encryptionType(0) == WIFI_AUTH_WPA_PSK) ? "WPA" :
                (WiFi.encryptionType(0) == WIFI_AUTH_WPA2_PSK) ? "WPA2" :
                (WiFi.encryptionType(0) == WIFI_AUTH_WPA_WPA2_PSK) ? "WPA/WPA2" :
                (WiFi.encryptionType(0) == WIFI_AUTH_WPA2_ENTERPRISE) ? "WPA2-Enterprise" :
                (WiFi.encryptionType(0) == WIFI_AUTH_WPA3_PSK) ? "WPA3" : "Unknown");
  Serial.println("+------------------------------------+");
}

bool NetworkManager::checkConnection() {
  isConnected = (WiFi.status() == WL_CONNECTED);
  return isConnected;
}

String NetworkManager::getIP() {
  return WiFi.localIP().toString();
}

int NetworkManager::getRSSI() {
  return WiFi.RSSI();
}