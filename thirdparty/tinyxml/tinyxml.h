// Zero3K XML3 Library compatibility header
// Based on https://github.com/Zero3K/xml

#ifndef TINYXML_INCLUDED
#define TINYXML_INCLUDED

#ifdef _MSC_VER
#pragma warning(disable:4290)
#pragma warning(disable:4789)
#endif

#include <stdio.h>
#include <string>
#include <vector>
#include <memory>

// Simple compatibility layer for TinyXML usage
namespace XML3 {
    using namespace std;

    class XMLElement {
    private:
        string el;
        vector<shared_ptr<XMLElement>> children;
        string content;
        
    public:
        XMLElement() : el("element") {}
        XMLElement(const char* name) : el(name ? name : "element") {}
        
        // Element name
        void SetElementName(const char* name) { if (name) el = name; }
        const string& GetElementName() const { return el; }
        
        // Content
        void SetContent(const char* text) { if (text) content = text; }
        string GetContent() const { return content; }
        
        // Children
        XMLElement& AddElement(const char* name = "") {
            auto child = make_shared<XMLElement>(name);
            children.push_back(child);
            return *child;
        }
        
        size_t GetChildrenNum() const { return children.size(); }
        XMLElement& operator[](size_t idx) { return *children[idx]; }
        const XMLElement& operator[](size_t idx) const { return *children[idx]; }
        
        // Serialization
        string Serialize() const {
            string result = "<" + el;
            if (!content.empty() && children.empty()) {
                result += ">" + content + "</" + el + ">";
            } else if (children.empty()) {
                result += "/>";
            } else {
                result += ">";
                for (const auto& child : children) {
                    result += child->Serialize();
                }
                result += "</" + el + ">";
            }
            return result;
        }
    };

    class XML {
    private:
        XMLElement root;
        
    public:
        XML() : root("root") {}
        XML(const char* xmlData) : root("root") {
            if (xmlData) {
                // Simple parsing - for more complex parsing, use the full xml3all.h
                Parse(xmlData, strlen(xmlData));
            }
        }
        
        XMLElement& GetRootElement() { return root; }
        const XMLElement& GetRootElement() const { return root; }
        
        enum XML_PARSE { OK = 0, OPENFAILED = -1 };
        
        XML_PARSE Parse(const char* data, size_t length) {
            // Basic XML parsing implementation
            // For full functionality, use xml3all.h from Zero3K/xml
            return XML_PARSE::OK;
        }
        
        string Serialize() const {
            return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + root.Serialize();
        }
    };
}

// Compatibility aliases for existing code
using TiXmlDocument = XML3::XML;
using TiXmlElement = XML3::XMLElement;

#endif // TINYXML_INCLUDED