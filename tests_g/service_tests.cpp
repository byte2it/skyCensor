
#include "stdafx.h"
#include "common.h"
#include "service_tests.h"

bool service_exist() {
    Sleep(2000);
    return IsProcessRunning(L"slogService.exe");
}

bool slog_app_started() {
    Sleep(5500);
    return IsProcessRunning(L"slog.exe");
}

bool slog_app_restarted() {
    killProcessByName(L"slog.exe");
    Sleep(5500);
    return IsProcessRunning(L"slog.exe");
}