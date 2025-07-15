// Metakit Database Interface Compatibility Header
// Based on https://github.com/Zero3K/metakit

#ifndef MK4_INCLUDED
#define MK4_INCLUDED

#include <string>
#include <vector>

// Basic compatibility layer for SQLite usage in benliud
namespace mk4 {
    using namespace std;

    // Basic storage interface
    class Storage {
    private:
        string filename;
        
    public:
        Storage() {}
        Storage(const char* fname) : filename(fname ? fname : "") {}
        
        bool IsValid() const { return !filename.empty(); }
        
        // Basic operations
        bool Open(const char* fname) { 
            if (fname) filename = fname;
            return true; 
        }
        
        void Close() { filename.clear(); }
        
        bool Execute(const char* sql) { return true; }
        
        // For compatibility with SQLite interface
        int GetLastError() const { return 0; }
        const char* GetErrorMessage() const { return "No error"; }
    };

    // Basic view interface
    class View {
    private:
        vector<vector<string>> data;
        
    public:
        View() {}
        
        size_t GetSize() const { return data.size(); }
        void SetSize(size_t size) { data.resize(size); }
        
        // Row access
        vector<string>& operator[](size_t index) { return data[index]; }
        const vector<string>& operator[](size_t index) const { return data[index]; }
        
        void Add(const vector<string>& row) { data.push_back(row); }
        void Clear() { data.clear(); }
    };
}

// SQLite compatibility interface
extern "C" {
    typedef struct sqlite3 sqlite3;
    typedef struct sqlite3_stmt sqlite3_stmt;
    
    // Basic SQLite API compatibility
    int sqlite3_open(const char* filename, sqlite3** ppDb);
    int sqlite3_close(sqlite3* db);
    int sqlite3_exec(sqlite3* db, const char* sql, int (*callback)(void*,int,char**,char**), void* arg, char** errmsg);
    int sqlite3_prepare_v2(sqlite3* db, const char* sql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail);
    int sqlite3_step(sqlite3_stmt* pStmt);
    int sqlite3_finalize(sqlite3_stmt* pStmt);
    const char* sqlite3_column_text(sqlite3_stmt* pStmt, int iCol);
    int sqlite3_column_int(sqlite3_stmt* pStmt, int iCol);
    const char* sqlite3_errmsg(sqlite3* db);
}

#define SQLITE_OK     0
#define SQLITE_ROW    100
#define SQLITE_DONE   101

#endif // MK4_INCLUDED