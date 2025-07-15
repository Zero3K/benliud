/*
Embedded minimal SQLite implementation
Simplified database functionality for benliud
This is a stub implementation - for production use, use SQLite amalgamation
*/

#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Minimal structures
struct sqlite3 {
    char *filename;
    int is_open;
    char *last_error;
};

struct sqlite3_stmt {
    sqlite3 *db;
    char *sql;
    int step_count;
    int is_done;
};

// Static error message
static char error_buffer[256];

int sqlite3_open(const char *filename, sqlite3 **ppDb)
{
    if (!filename || !ppDb) return SQLITE_ERROR;
    
    sqlite3 *db = (sqlite3*)malloc(sizeof(sqlite3));
    if (!db) return SQLITE_NOMEM;
    
    db->filename = (char*)malloc(strlen(filename) + 1);
    if (!db->filename) {
        free(db);
        return SQLITE_NOMEM;
    }
    
    strcpy(db->filename, filename);
    db->is_open = 1;
    db->last_error = NULL;
    
    *ppDb = db;
    return SQLITE_OK;
}

int sqlite3_close(sqlite3 *db)
{
    if (!db) return SQLITE_ERROR;
    
    if (db->filename) free(db->filename);
    if (db->last_error) free(db->last_error);
    free(db);
    
    return SQLITE_OK;
}

int sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg)
{
    if (!db || !sql) return SQLITE_ERROR;
    
    // Stub implementation - just pretend to execute
    if (strstr(sql, "CREATE TABLE") || strstr(sql, "INSERT") || strstr(sql, "UPDATE") || strstr(sql, "DELETE")) {
        return SQLITE_OK; // Pretend write operations succeed
    }
    
    if (strstr(sql, "SELECT")) {
        // Pretend empty result set
        return SQLITE_OK;
    }
    
    return SQLITE_OK;
}

int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail)
{
    if (!db || !zSql || !ppStmt) return SQLITE_ERROR;
    
    sqlite3_stmt *stmt = (sqlite3_stmt*)malloc(sizeof(sqlite3_stmt));
    if (!stmt) return SQLITE_NOMEM;
    
    int sql_len = (nByte < 0) ? (int)strlen(zSql) : nByte;
    stmt->sql = (char*)malloc(sql_len + 1);
    if (!stmt->sql) {
        free(stmt);
        return SQLITE_NOMEM;
    }
    
    strncpy(stmt->sql, zSql, sql_len);
    stmt->sql[sql_len] = '\0';
    stmt->db = db;
    stmt->step_count = 0;
    stmt->is_done = 0;
    
    *ppStmt = stmt;
    return SQLITE_OK;
}

int sqlite3_step(sqlite3_stmt *pStmt)
{
    if (!pStmt) return SQLITE_ERROR;
    
    if (pStmt->is_done) return SQLITE_DONE;
    
    pStmt->step_count++;
    
    // For SELECT statements, pretend we have no rows
    if (strstr(pStmt->sql, "SELECT")) {
        pStmt->is_done = 1;
        return SQLITE_DONE;
    }
    
    // For other statements, pretend they execute successfully
    pStmt->is_done = 1;
    return SQLITE_DONE;
}

int sqlite3_finalize(sqlite3_stmt *pStmt)
{
    if (!pStmt) return SQLITE_ERROR;
    
    if (pStmt->sql) free(pStmt->sql);
    free(pStmt);
    
    return SQLITE_OK;
}

int sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void(*xDel)(void*))
{
    // Stub implementation
    return SQLITE_OK;
}

int sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue)
{
    // Stub implementation
    return SQLITE_OK;
}

const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int iCol)
{
    // Stub implementation - return empty string
    return (const unsigned char*)"";
}

int sqlite3_column_int(sqlite3_stmt *pStmt, int iCol)
{
    // Stub implementation - return 0
    return 0;
}

const char *sqlite3_errmsg(sqlite3 *db)
{
    if (!db) return "Invalid database handle";
    return db->last_error ? db->last_error : "No error";
}

void sqlite3_free(void *p)
{
    if (p) free(p);
}