// Zero3K XML Library Direct Implementation
// Based on https://github.com/Zero3K/xml

#include "tinyxml.h"
#include <sstream>
#include <cctype>

namespace XML3 {

void XMLDocument::skipWhitespace(const string& xml, size_t& pos) {
    while(pos < xml.length() && isspace(xml[pos])) {
        pos++;
    }
}

string XMLDocument::parseText(const string& xml, size_t& pos) {
    string text;
    while(pos < xml.length() && xml[pos] != '<') {
        text += xml[pos];
        pos++;
    }
    
    // Trim whitespace
    size_t start = text.find_first_not_of(" \t\n\r");
    size_t end = text.find_last_not_of(" \t\n\r");
    if(start != string::npos && end != string::npos) {
        return text.substr(start, end - start + 1);
    }
    return "";
}

shared_ptr<XMLNode> XMLDocument::parseElement(const string& xml, size_t& pos) {
    skipWhitespace(xml, pos);
    
    if(pos >= xml.length() || xml[pos] != '<') {
        return nullptr;
    }
    
    pos++; // skip '<'
    
    // Check for closing tag
    if(pos < xml.length() && xml[pos] == '/') {
        return nullptr;
    }
    
    // Parse element name
    string elementName;
    while(pos < xml.length() && xml[pos] != '>' && xml[pos] != ' ' && xml[pos] != '\t') {
        elementName += xml[pos];
        pos++;
    }
    
    auto element = make_shared<XMLNode>(elementName);
    
    // Skip attributes for now (simplified parser)
    while(pos < xml.length() && xml[pos] != '>') {
        pos++;
    }
    
    if(pos >= xml.length()) {
        hasError = true;
        errorMsg = "Unexpected end of XML";
        return nullptr;
    }
    
    pos++; // skip '>'
    
    // Parse content and children
    while(pos < xml.length()) {
        skipWhitespace(xml, pos);
        
        if(pos >= xml.length()) break;
        
        if(xml[pos] == '<') {
            if(pos + 1 < xml.length() && xml[pos + 1] == '/') {
                // Closing tag
                pos++; // skip '<'
                pos++; // skip '/'
                
                // Skip to '>'
                while(pos < xml.length() && xml[pos] != '>') {
                    pos++;
                }
                pos++; // skip '>'
                break;
            } else {
                // Child element
                auto child = parseElement(xml, pos);
                if(child) {
                    element->addChild(child);
                }
            }
        } else {
            // Text content
            string text = parseText(xml, pos);
            if(!text.empty()) {
                element->setText(text);
            }
        }
    }
    
    return element;
}

bool XMLDocument::parse(const string& xmlContent) {
    hasError = false;
    errorMsg = "";
    
    size_t pos = 0;
    
    // Skip XML declaration if present
    skipWhitespace(xmlContent, pos);
    if(pos < xmlContent.length() - 5 && xmlContent.substr(pos, 5) == "<?xml") {
        while(pos < xmlContent.length() && xmlContent.substr(pos, 2) != "?>") {
            pos++;
        }
        if(pos < xmlContent.length()) {
            pos += 2; // skip "?>"
        }
    }
    
    root = parseElement(xmlContent, pos);
    return root != nullptr && !hasError;
}

} // namespace XML3