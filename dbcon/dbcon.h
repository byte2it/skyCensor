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

#ifndef DBCON_DBCON_H_
#define DBCON_DBCON_H_

#ifdef DBCON_EXPORTS
#define DBCON_API __declspec(dllexport)
#else
#define DBCON_API __declspec(dllimport)
#endif

#include "CppSQLite3.h"
#include <vector>
#include <string>

namespace dbcon {

class UserStat {
  double date;
  char status;
 public :
  UserStat(double d, char s):date(d), status(s) {}
  ~UserStat(){}

  const double getDate()const {return date; }
  const char getStatus()const {return status;}
};

class DBCON_API CDatabase {
 public:
  CDatabase(){}
  explicit CDatabase(const char* dbname);

  bool init(const char* dbname);

  void ExecQuery(const char* query);
  void GetUsers(std::vector<std::pair<std::wstring, std::wstring>>& users);
  unsigned int GetMaxUserId();
  unsigned int GetMaxMsgId();
  void ExecQuery16(const void** query);
  void GetUserStatistic(const wchar_t* user, std::vector<UserStat>& stat);
  bool TableExists(const wchar_t* table);
 private:
  const char* m_dbname;
};
};  // namespace dbcon

#endif  // DBCON_DBCON_H_
