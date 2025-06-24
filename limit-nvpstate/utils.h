#pragma once

#include <string>
#include <Windows.h>

std::string getBasePath(std::string path);
std::string getProgramPath();
bool isAddedToStartup();
int addToStartup(bool isEnabled);
std::string toLower(std::string str);
std::string getBaseName(std::string& fullPath);
std::string wStringToString(const std::wstring& wstr);
std::string getProcessNameByPID(DWORD pid);