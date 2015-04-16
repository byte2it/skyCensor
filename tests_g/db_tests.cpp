#include "stdafx.h"
#include "db_tests.h"

static dbcon::CDatabase db;

bool db_init() {
    return db.init("test_db1.tdb");
}

bool create_tables() {
    db.ExecQuery("create table statuses(name char(16), statusid integer);");
    db.ExecQuery("create table user_statuses(time double, user char(32), statusid integer);");
    return (db.TableExists(L"statuses") && db.TableExists(L"user_statuses"));
}

