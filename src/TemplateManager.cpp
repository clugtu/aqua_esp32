#include "TemplateManager.h"

TemplateManager::TemplateManager(bool enableCache) : cacheEnabled(enableCache) {
}

String TemplateManager::loadTemplate(const String& templateName) {
    // Check cache first
    if (cacheEnabled && templateCache.find(templateName) != templateCache.end()) {
        return templateCache[templateName];
    }
    
    String templatePath = getTemplatePath(templateName);
    
    if (!SPIFFS.exists(templatePath)) {
        Serial.println("Template not found: " + templatePath);
        return "";
    }
    
    File file = SPIFFS.open(templatePath, "r");
    if (!file) {
        Serial.println("Failed to open template: " + templatePath);
        return "";
    }
    
    String content = file.readString();
    file.close();
    
    // Cache the template if caching is enabled
    if (cacheEnabled) {
        templateCache[templateName] = content;
    }
    
    return content;
}

String TemplateManager::processTemplate(const String& templateContent, const std::map<String, String>& variables) {
    String result = templateContent;
    
    // Replace all variables in the format {{VARIABLE_NAME}}
    for (const auto& pair : variables) {
        String placeholder = "{{" + pair.first + "}}";
        result.replace(placeholder, pair.second);
    }
    
    return result;
}

String TemplateManager::renderTemplate(const String& templateName, const std::map<String, String>& variables) {
    String templateContent = loadTemplate(templateName);
    if (templateContent.length() == 0) {
        return "";
    }
    
    return processTemplate(templateContent, variables);
}

void TemplateManager::clearCache() {
    templateCache.clear();
}

bool TemplateManager::templateExists(const String& templateName) {
    String templatePath = getTemplatePath(templateName);
    return SPIFFS.exists(templatePath);
}

String TemplateManager::getTemplatePath(const String& templateName) {
    return "/templates/" + templateName + ".html";
}