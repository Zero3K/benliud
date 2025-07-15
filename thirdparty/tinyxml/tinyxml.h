/*
TinyXML - Embedded minimal XML parser
This is a simplified, embedded version for benliud project
Public domain implementation
*/

#ifndef TINYXML_INCLUDED
#define TINYXML_INCLUDED

#include <string>
#include <vector>

class TiXmlBase
{
public:
    TiXmlBase() {}
    virtual ~TiXmlBase() {}
    
protected:
    static void SkipWhiteSpace(const char** p);
    static bool IsWhiteSpace(char c);
};

class TiXmlAttribute : public TiXmlBase
{
public:
    TiXmlAttribute() : name_(), value_() {}
    TiXmlAttribute(const std::string& _name, const std::string& _value) : name_(_name), value_(_value) {}
    
    const char* Name() const { return name_.c_str(); }
    const char* Value() const { return value_.c_str(); }
    
    void SetName(const std::string& _name) { name_ = _name; }
    void SetValue(const std::string& _value) { value_ = _value; }
    
private:
    std::string name_;
    std::string value_;
};

class TiXmlNode : public TiXmlBase
{
public:
    enum NodeType
    {
        UNKNOWN = 0,
        ELEMENT,
        TEXT,
        COMMENT,
        DOCUMENT
    };
    
    TiXmlNode() : type_(UNKNOWN), parent_(0) {}
    virtual ~TiXmlNode();
    
    NodeType Type() const { return type_; }
    
    TiXmlNode* FirstChild() const;
    TiXmlNode* FirstChild(const char* value) const;
    TiXmlNode* NextSibling() const;
    TiXmlNode* NextSibling(const char* value) const;
    
    virtual const char* Value() const { return value_.c_str(); }
    virtual void SetValue(const char* _value) { value_ = _value; }
    
protected:
    NodeType type_;
    std::string value_;
    TiXmlNode* parent_;
    std::vector<TiXmlNode*> children_;
};

class TiXmlElement : public TiXmlNode
{
    friend class TiXmlDocument;  // Allow access to protected members
public:
    TiXmlElement(const char* _value);
    virtual ~TiXmlElement() {}
    
    const char* Attribute(const char* name) const;
    void SetAttribute(const char* name, const char* value);
    
    const char* GetText() const;
    
private:
    std::vector<TiXmlAttribute> attributes_;
};

class TiXmlText : public TiXmlNode
{
    friend class TiXmlDocument;  // Allow access to protected members
public:
    TiXmlText(const char* text);
    virtual ~TiXmlText() {}
};

class TiXmlDocument : public TiXmlNode
{
    friend class TiXmlNode;  // Allow access to protected members
public:
    TiXmlDocument();
    TiXmlDocument(const char* documentName);
    virtual ~TiXmlDocument() {}
    
    bool LoadFile(const char* filename);
    bool Parse(const char* p);
    
    TiXmlElement* RootElement() const;
    
private:
    bool ParseDocument(const char* p);
    TiXmlNode* ParseElement(const char** p, TiXmlNode* parent);
};

#endif // TINYXML_INCLUDED