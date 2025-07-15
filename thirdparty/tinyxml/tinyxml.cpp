/*
TinyXML - Embedded minimal XML parser implementation
This is a simplified, embedded version for benliud project  
Public domain implementation
*/

#include "tinyxml.h"
#include <fstream>
#include <cstring>
#include <cctype>

// TiXmlBase implementation
void TiXmlBase::SkipWhiteSpace(const char** p)
{
    while (*p && IsWhiteSpace(**p))
        (*p)++;
}

bool TiXmlBase::IsWhiteSpace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

// TiXmlNode implementation
TiXmlNode::~TiXmlNode()
{
    for (size_t i = 0; i < children_.size(); ++i)
        delete children_[i];
}

TiXmlNode* TiXmlNode::FirstChild() const
{
    return children_.empty() ? 0 : children_[0];
}

TiXmlNode* TiXmlNode::FirstChild(const char* value) const
{
    for (size_t i = 0; i < children_.size(); ++i)
    {
        if (children_[i]->value_ == value)
            return children_[i];
    }
    return 0;
}

TiXmlNode* TiXmlNode::NextSibling() const
{
    if (!parent_) return 0;
    
    for (size_t i = 0; i < parent_->children_.size() - 1; ++i)
    {
        if (parent_->children_[i] == this)
            return parent_->children_[i + 1];
    }
    return 0;
}

TiXmlNode* TiXmlNode::NextSibling(const char* value) const
{
    TiXmlNode* node = NextSibling();
    while (node)
    {
        if (node->value_ == value)
            return node;
        node = node->NextSibling();
    }
    return 0;
}

// TiXmlElement implementation
TiXmlElement::TiXmlElement(const char* _value)
{
    type_ = ELEMENT;
    value_ = _value;
}

const char* TiXmlElement::Attribute(const char* name) const
{
    for (size_t i = 0; i < attributes_.size(); ++i)
    {
        if (strcmp(attributes_[i].Name(), name) == 0)
            return attributes_[i].Value();
    }
    return 0;
}

void TiXmlElement::SetAttribute(const char* name, const char* value)
{
    for (size_t i = 0; i < attributes_.size(); ++i)
    {
        if (strcmp(attributes_[i].Name(), name) == 0)
        {
            attributes_[i].SetValue(value);
            return;
        }
    }
    attributes_.push_back(TiXmlAttribute(name, value));
}

const char* TiXmlElement::GetText() const
{
    for (size_t i = 0; i < children_.size(); ++i)
    {
        if (children_[i]->Type() == TEXT)
            return children_[i]->Value();
    }
    return 0;
}

// TiXmlText implementation
TiXmlText::TiXmlText(const char* text)
{
    type_ = TEXT;
    value_ = text;
}

// TiXmlDocument implementation
TiXmlDocument::TiXmlDocument()
{
    type_ = DOCUMENT;
}

TiXmlDocument::TiXmlDocument(const char* documentName)
{
    type_ = DOCUMENT;
    value_ = documentName;
}

bool TiXmlDocument::LoadFile(const char* filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    return Parse(content.c_str());
}

bool TiXmlDocument::Parse(const char* p)
{
    // Clear existing children
    for (size_t i = 0; i < children_.size(); ++i)
        delete children_[i];
    children_.clear();
    
    return ParseDocument(p);
}

TiXmlElement* TiXmlDocument::RootElement() const
{
    for (size_t i = 0; i < children_.size(); ++i)
    {
        if (children_[i]->Type() == ELEMENT)
            return static_cast<TiXmlElement*>(children_[i]);
    }
    return 0;
}

bool TiXmlDocument::ParseDocument(const char* p)
{
    SkipWhiteSpace(&p);
    
    // Skip XML declaration if present
    if (strncmp(p, "<?xml", 5) == 0)
    {
        while (*p && strncmp(p, "?>", 2) != 0)
            p++;
        if (*p) p += 2;
        SkipWhiteSpace(&p);
    }
    
    // Parse root element
    TiXmlNode* root = ParseElement(&p, this);
    if (root)
    {
        children_.push_back(root);
        return true;
    }
    
    return false;
}

TiXmlNode* TiXmlDocument::ParseElement(const char** p, TiXmlNode* parent)
{
    SkipWhiteSpace(p);
    
    if (**p != '<')
        return 0;
    
    (*p)++; // Skip '<'
    
    // Read element name
    const char* start = *p;
    while (**p && **p != '>' && **p != ' ' && **p != '\t' && **p != '\n' && **p != '\r')
        (*p)++;
    
    if (*p == start)
        return 0;
    
    std::string elementName(start, *p);
    TiXmlElement* element = new TiXmlElement(elementName.c_str());
    element->parent_ = parent;
    
    // Parse attributes
    SkipWhiteSpace(p);
    while (**p && **p != '>')
    {
        if (**p == '/')
        {
            (*p)++;
            if (**p == '>')
            {
                (*p)++;
                return element; // Self-closing tag
            }
        }
        
        // Parse attribute name
        start = *p;
        while (**p && **p != '=' && !IsWhiteSpace(**p))
            (*p)++;
        
        if (*p == start)
            break;
        
        std::string attrName(start, *p);
        
        SkipWhiteSpace(p);
        if (**p != '=')
            break;
        (*p)++; // Skip '='
        SkipWhiteSpace(p);
        
        // Parse attribute value
        char quote = **p;
        if (quote != '"' && quote != '\'')
            break;
        (*p)++; // Skip opening quote
        
        start = *p;
        while (**p && **p != quote)
            (*p)++;
        
        std::string attrValue(start, *p);
        if (**p) (*p)++; // Skip closing quote
        
        element->SetAttribute(attrName.c_str(), attrValue.c_str());
        SkipWhiteSpace(p);
    }
    
    if (**p == '>')
        (*p)++; // Skip '>'
    
    // Parse element content
    SkipWhiteSpace(p);
    while (**p && strncmp(*p, "</", 2) != 0)
    {
        if (**p == '<')
        {
            // Child element
            TiXmlNode* child = ParseElement(p, element);
            if (child)
                element->children_.push_back(child);
        }
        else
        {
            // Text content
            start = *p;
            while (**p && **p != '<')
                (*p)++;
            
            if (*p > start)
            {
                std::string text(start, *p);
                TiXmlText* textNode = new TiXmlText(text.c_str());
                textNode->parent_ = element;
                element->children_.push_back(textNode);
            }
        }
        SkipWhiteSpace(p);
    }
    
    // Skip closing tag
    if (strncmp(*p, "</", 2) == 0)
    {
        *p += 2;
        while (**p && **p != '>')
            (*p)++;
        if (**p) (*p)++;
    }
    
    return element;
}