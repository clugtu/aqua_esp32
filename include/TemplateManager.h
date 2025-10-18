#ifndef TEMPLATE_MANAGER_H
#define TEMPLATE_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <map>

class TemplateManager {
private:
    std::map<String, String> templateCache;
    bool cacheEnabled;

public:
    TemplateManager(bool enableCache = true);
    
    // Load template from SPIFFS
    String loadTemplate(const String& templateName);
    
    // Replace variables in template ({{VARIABLE}} format)
    String processTemplate(const String& templateContent, const std::map<String, String>& variables);
    
    // Load and process template in one call
    String renderTemplate(const String& templateName, const std::map<String, String>& variables);
    
    // Clear template cache
    void clearCache();
    
    // Check if template exists
    bool templateExists(const String& templateName);
    
    // Get template file path
    String getTemplatePath(const String& templateName);
};

#endif