/*
    Copyright (c) 2015 Denys Ponomarenko (denys.ponomarenko@gmail.com).

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "dbcon.h"

namespace dbcon {

typedef struct _STATUS {
    short        key;
    const wchar_t*  desc;
}STATUS;

STATUS statusesArray[] = {
    {0, L"Offline"},
    {1, L"Online"},
    {2, L"Away"},
    {3, L"Not available"},
    {4, L"Do not disturb"},
    {5, L"Skype out"},
    {6, L"Skype ME"},
    {-1, L"Uknown"}
};

static CppSQLite3DB db;

void ansi2Unicode(wchar_t* out, const char* in, size_t max) {
    mbstowcs(out, in, MB_CUR_MAX);
}

CDatabase::CDatabase(const char* dbname)
: m_dbname(dbname) {
    try {
        db.open(dbname);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
}

bool CDatabase::init(const char* dbname) {
    bool res = true;
    try {
        db.open(dbname);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
        res =  false;
    }
    return res;
}

void CDatabase::ExecQuery(const char* query) {
    try {
        db.execDML(query);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
}

void CDatabase::ExecQuery16(const void** query) {
    try {
        db.execQuery(query);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
}

bool CDatabase::TableExists(const wchar_t* table) {
    wchar_t szSQL[256] = {0};
    wsprintf(szSQL,
            L"select count(*) from sqlite_master where type='table' " \
            L"and name='%s'", table);

    int nRet = 0;

    try {
         nRet = db.execScalar((const void**)szSQL);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
    return (nRet > 0);
}

void CDatabase::GetUsers(
  std::vector<std::pair<std::wstring,
  std::wstring>>& users) {
    const wchar_t* pQ = L"select * from users order by name;";

    try {
        CppSQLite3Query q = db.execQuery((const void**)pQ);

        while (!q.eof()) {
            std::wstring sId((wchar_t*)q.getStringField16(0));
            std::wstring sName((wchar_t*)q.getStringField16(1));
            users.push_back(std::pair<std::wstring, std::wstring>(sId, sName));
            q.nextRow();
        }
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
}

unsigned int CDatabase::GetMaxUserId() {
    const wchar_t* pQ = L"SELECT MAX(userid) FROM users;";

    unsigned int res = 0;
    try {
        res = db.execScalar((const void**)pQ);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
    return res;
}

unsigned int CDatabase::GetMaxMsgId() {
    const wchar_t* pQ = L"SELECT MAX(msgid) FROM messages;";

    unsigned int res = 0;
    try {
        res = db.execScalar((const void**)pQ);
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }

    return res;
}

void CDatabase::GetUserStatistic(const wchar_t* user,
                                 std::vector<UserStat>& stat) {
    wchar_t query[256] = {0};
    wsprintf(query,
        L"SELECT * FROM user_statuses JOIN users ON user_statuses.user = "  \
         L"users.handle WHERE user_statuses.user = " \
         L"'%s' order by user_statuses.time;",
         user);

    try {
        CppSQLite3Query q = db.execQuery((const void**)(const wchar_t*)query);

        while (!q.eof()) {
            UserStat st(q.getFloatField(0), q.getIntField(2));
            stat.push_back(st);
            q.nextRow();
        }
    }
    catch (CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << ":" << e.errorMessage() << std::endl;
    }
}

}  // end of namespace dbcon

