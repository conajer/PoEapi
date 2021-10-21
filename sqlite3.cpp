/*
* sqlite3.cpp, 6/11/2021 12:08 AM
*/

#include <sqlite3.h>

class sqlite3 : public AhkObj {
public:

    struct sqlite3* db;

    sqlite3() : db(nullptr) {
        add_method(L"open", (sqlite3*)this, (MethodType)&sqlite3::open, AhkInt, ParamList{AhkWString});
        add_method(L"close", (sqlite3*)this, (MethodType)&sqlite3::close, AhkInt);
        add_method(L"exec", (sqlite3*)this, (MethodType)&sqlite3::exec, AhkObject, ParamList{AhkWString});
        add_method(L"changes", (sqlite3*)this, (MethodType)&sqlite3::changes, AhkInt);
        add_method(L"totalChanges", (sqlite3*)this, (MethodType)&sqlite3::total_changes, AhkInt);
        add_method(L"version", (sqlite3*)this, (MethodType)&sqlite3::version, AhkString);
    }

    int open(const wchar_t* filename) {
        sqlite3_close(db);
        return sqlite3_open16(filename, &db);
    }

    int close() {
        return sqlite3_close(db);
    }

    AhkObjRef* exec(const wchar_t* sql) {
        sqlite3_stmt* stmt;
        AhkTempObj result;

        while (sql && *sql) {
            int rc = sqlite3_prepare16_v2(db, sql, -1, &stmt, (const void**)&sql);
            if (rc != SQLITE_OK) {
                result.__set(L"errcode", rc, AhkInt,
                             L"errmsg", sqlite3_errstr(rc), AhkString, nullptr);
                return result;
            }

            int cols = sqlite3_column_count(stmt);
            wchar_t** col_names = (wchar_t**)sqlite3_malloc(cols * sizeof(wchar_t**));
            for (int i = 0; i < cols; ++i)
                col_names[i] = (wchar_t*)sqlite3_column_name16(stmt, i);

            while (1) {
                rc = sqlite3_step(stmt);
                if (rc == SQLITE_DONE || rc != SQLITE_ROW)
                    break;

                AhkObj row;
                for (int i = 0; i < cols; ++i) {
                    const wchar_t *value = (const wchar_t*)sqlite3_column_text16(stmt, i);
                    if (value && value[0] != L'\0')
                        row.__set(col_names[i], value, AhkWString, nullptr);
                }
                result.__set(L"", (AhkObjRef*)row, AhkObject, nullptr);
            }
            sqlite3_free(col_names);
            rc = sqlite3_finalize(stmt);
            if (rc != SQLITE_OK) {
                result.__set(L"errcode", rc, AhkInt,
                             L"errmsg", sqlite3_errmsg16(db), AhkWString, nullptr);
                break;
            }
        }

        return result;
    }

    int changes() {
        return sqlite3_changes(db);
    }

    int total_changes() {
        return sqlite3_total_changes(db);
    }

    const char* version() {
        return sqlite3_libversion();
    }
};
