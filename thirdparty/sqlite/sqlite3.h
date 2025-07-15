/*
Embedded minimal SQLite interface
Simplified database functionality for benliud
This is a minimal implementation - for production use, use SQLite amalgamation
*/

#ifndef EMBEDDED_SQLITE3_H
#define EMBEDDED_SQLITE3_H

#ifdef __cplusplus
extern "C" {
#endif

#define SQLITE_OK           0   /* Successful result */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_NOTFOUND    12   /* Unknown operation */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

// Basic SQLite API subset
int sqlite3_open(const char *filename, sqlite3 **ppDb);
int sqlite3_close(sqlite3 *db);
int sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg);
int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
int sqlite3_step(sqlite3_stmt *pStmt);
int sqlite3_finalize(sqlite3_stmt *pStmt);
int sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void(*xDel)(void*));
int sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue);
const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int iCol);
int sqlite3_column_int(sqlite3_stmt *pStmt, int iCol);
const char *sqlite3_errmsg(sqlite3 *db);
void sqlite3_free(void *p);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_SQLITE3_H