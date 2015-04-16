
#pragma once

#include <string>

bool IsProcessRunning(const wchar_t *processName);
void killProcessByName(const wchar_t *filename);

std::string GetUserAppFolderPath();