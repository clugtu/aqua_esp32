#pragma once

#include "Config.h"

// ========================================
// ICON POLICY ENFORCEMENT
// ========================================
// This header ensures ZERO icons are ever used in the system
// All output must be pure ASCII for maximum compatibility

class IconPolicy {
public:
  // Static methods to get safe ASCII representations
  static const char* getTempUnit() { return "C"; }           // Instead of °C
  static const char* getDegreeSymbol() { return ""; }        // No degree symbol
  static const char* getWifiIndicator() { return "WiFi"; }   // Instead of WiFi icon
  static const char* getBulletPoint() { return "- "; }       // Simple dash
  static const char* getCheckMark() { return "[OK]"; }       // Instead of ✓
  static const char* getErrorMark() { return "[ERR]"; }      // Instead of ✗
  static const char* getWarningMark() { return "[WARN]"; }   // Instead of ⚠
  static const char* getInfoMark() { return "[INFO]"; }      // Instead of ℹ
  
  // Safe separator lines (ASCII only)
  static const char* getSeparator() { return "========================================"; }
  static const char* getShortSeparator() { return "------------------------"; }
  static const char* getDashLine() { return "----------------------------"; }
  
  // Status indicators (text only)
  static const char* getOnlineStatus() { return "ONLINE"; }
  static const char* getOfflineStatus() { return "OFFLINE"; }
  static const char* getConnectedStatus() { return "CONNECTED"; }
  static const char* getDisconnectedStatus() { return "DISCONNECTED"; }
  
  // Validation method to check if a string contains problematic characters
  static bool isAsciiOnly(const String& text) {
    for (int i = 0; i < text.length(); i++) {
      if (text.charAt(i) > 127) {  // Non-ASCII character detected
        return false;
      }
    }
    return true;
  }
  
  // Method to strip non-ASCII characters if any accidentally get through
  static String ensureAsciiOnly(const String& text) {
    String result = "";
    for (int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (c <= 127) {  // Keep only ASCII characters
        result += c;
      }
    }
    return result;
  }
};

// Convenience macros for common use cases
#define SAFE_TEMP_UNIT        IconPolicy::getTempUnit()
#define SAFE_WIFI_INDICATOR   IconPolicy::getWifiIndicator()
#define SAFE_BULLET           IconPolicy::getBulletPoint()
#define SAFE_OK               IconPolicy::getCheckMark()
#define SAFE_ERROR            IconPolicy::getErrorMark()
#define SAFE_SEPARATOR        IconPolicy::getSeparator()