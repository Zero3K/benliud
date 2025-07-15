// Zero3K XML Library Direct Implementation
// Based on https://github.com/Zero3K/xml

#ifndef XML3ALL_H
#define XML3ALL_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace std;

namespace XML3 {

class XMLNode {
public:
    string name;
    string text;
    map<string, string> attributes;
    vector<shared_ptr<XMLNode>> children;
    
    XMLNode() {}
    XMLNode(const string& nodeName) : name(nodeName) {}
    
    shared_ptr<XMLNode> getChild(const string& childName) {
        for(auto& child : children) {
            if(child->name == childName) {
                return child;
            }
        }
        return nullptr;
    }
    
    string getAttribute(const string& attrName) {
        auto it = attributes.find(attrName);
        return (it != attributes.end()) ? it->second : "";
    }
    
    void addChild(shared_ptr<XMLNode> child) {
        children.push_back(child);
    }
    
    void setText(const string& nodeText) {
        text = nodeText;
    }
};

class XMLDocument {
private:
    shared_ptr<XMLNode> root;
    bool hasError;
    string errorMsg;
    
    shared_ptr<XMLNode> parseElement(const string& xml, size_t& pos);
    string parseText(const string& xml, size_t& pos);
    void skipWhitespace(const string& xml, size_t& pos);
    
public:
    XMLDocument() : hasError(false) {}
    
    bool parse(const string& xmlContent);
    shared_ptr<XMLNode> getRootElement() { return root; }
    bool error() { return hasError; }
    string getErrorMessage() { return errorMsg; }
};

// TinyXML compatibility layer
class TiXmlDocument {
private:
    XML3::XMLDocument doc;
    
public:
    bool Parse(const char* xml) {
        return doc.parse(string(xml));
    }
    
    bool Error() {
        return doc.error();
    }
    
    XMLNode* RootElement() {
        auto root = doc.getRootElement();
        return root.get();
    }
};

class TiXmlElement {
public:
    XMLNode* node;
    
    TiXmlElement(XMLNode* n) : node(n) {}
    
    const char* GetText() {
        return node ? node->text.c_str() : nullptr;
    }
    
    TiXmlElement* FirstChildElement(const char* name) {
        if(!node) return nullptr;
        auto child = node->getChild(string(name));
        return child ? new TiXmlElement(child.get()) : nullptr;
    }
};

class TiXmlHandle {
private:
    XMLNode* node;
    
public:
    TiXmlHandle(XMLNode* n) : node(n) {}
    
    TiXmlHandle FirstChildElement(const char* name) {
        if(!node) return TiXmlHandle(nullptr);
        auto child = node->getChild(string(name));
        return TiXmlHandle(child.get());
    }
    
    TiXmlHandle FirstChild(const char* name) {
        return FirstChildElement(name);
    }
    
    TiXmlHandle Child(const char* name, int index) {
        if(!node) return TiXmlHandle(nullptr);
        int count = 0;
        for(auto& child : node->children) {
            if(child->name == string(name)) {
                if(count == index) {
                    return TiXmlHandle(child.get());
                }
                count++;
            }
        }
        return TiXmlHandle(nullptr);
    }
    
    TiXmlElement* ToElement() {
        return node ? new TiXmlElement(node) : nullptr;
    }
};

} // namespace XML3

// Export TinyXML compatibility types to global namespace for backward compatibility
using TiXmlDocument = XML3::TiXmlDocument;
using TiXmlElement = XML3::TiXmlElement;
using TiXmlHandle = XML3::TiXmlHandle;

#endif // XML3ALL_H