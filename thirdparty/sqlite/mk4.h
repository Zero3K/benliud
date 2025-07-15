// Zero3K Metakit Database Implementation 
// Based on https://github.com/Zero3K/metakit

#ifndef MK4_INCLUDED
#define MK4_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace mk4 {
    using namespace std;

    // Forward declarations
    class View;
    class Row;
    class Property;

    // Property types
    enum PropertyType {
        StringProperty,
        IntProperty,
        LongProperty,
        FloatProperty,
        BinaryProperty
    };

    // Property definition
    class Property {
    private:
        string name;
        PropertyType type;
        
    public:
        Property(const string& propName, PropertyType propType) 
            : name(propName), type(propType) {}
        
        const string& GetName() const { return name; }
        PropertyType GetType() const { return type; }
    };

    // Row data
    class Row {
    private:
        map<string, string> data;
        
    public:
        Row() {}
        
        void SetString(const string& prop, const string& value) {
            data[prop] = value;
        }
        
        void SetInt(const string& prop, int value) {
            data[prop] = to_string(value);
        }
        
        void SetLong(const string& prop, long long value) {
            data[prop] = to_string(value);
        }
        
        string GetString(const string& prop) const {
            auto it = data.find(prop);
            return (it != data.end()) ? it->second : "";
        }
        
        int GetInt(const string& prop) const {
            string val = GetString(prop);
            return val.empty() ? 0 : atoi(val.c_str());
        }
        
        long long GetLong(const string& prop) const {
            string val = GetString(prop);
            return val.empty() ? 0 : atoll(val.c_str());
        }
        
        bool HasProperty(const string& prop) const {
            return data.find(prop) != data.end();
        }
    };

    // View (table)
    class View {
    private:
        string viewName;
        vector<Property> properties;
        vector<Row> rows;
        
    public:
        View() {}
        View(const string& name) : viewName(name) {}
        
        // Property management
        void AddProperty(const Property& prop) {
            properties.push_back(prop);
        }
        
        Property GetProperty(int index) const {
            return (index >= 0 && index < properties.size()) ? properties[index] : Property("", StringProperty);
        }
        
        int GetSize() const { return rows.size(); }
        
        // Row access
        Row GetAt(int index) const {
            return (index >= 0 && index < rows.size()) ? rows[index] : Row();
        }
        
        void SetAt(int index, const Row& row) {
            if (index >= 0 && index < rows.size()) {
                rows[index] = row;
            }
        }
        
        void Add(const Row& row) {
            rows.push_back(row);
        }
        
        void RemoveAt(int index) {
            if (index >= 0 && index < rows.size()) {
                rows.erase(rows.begin() + index);
            }
        }
        
        void SetSize(int size) {
            rows.resize(size);
        }
        
        // Simple search
        View Select(const string& whereClause) const {
            View result(viewName + "_filtered");
            result.properties = properties;
            
            // Simple implementation - for now just return all rows
            // In a real implementation, this would parse the whereClause
            result.rows = rows;
            return result;
        }
        
        const string& GetName() const { return viewName; }
    };

    // Storage (database)
    class Storage {
    private:
        string filename;
        map<string, View> views;
        bool isOpen;
        
    public:
        Storage() : isOpen(false) {}
        Storage(const string& fname) : filename(fname), isOpen(false) {
            Open(fname);
        }
        
        ~Storage() {
            Close();
        }
        
        bool Open(const string& fname);
        void Close();
        bool IsValid() const { return isOpen; }
        
        // View access
        View GetAs(const string& description);
        void SetAs(const string& description, const View& view) {
            views[description] = view;
        }
        
        // Transaction support
        void Commit();
        void Rollback() {
            // In a real implementation, this would restore from file
        }
        
        // Basic operations
        bool Execute(const string& command) {
            // Basic command execution
            return true;
        }
        
        int GetLastError() const { return 0; }
        const char* GetErrorMessage() const { return "No error"; }
    };

} // namespace mk4

#endif // MK4_INCLUDED