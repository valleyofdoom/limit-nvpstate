#include <QMessageBox>
#include <Shlwapi.h>
#include <Psapi.h>
#include <tlhelp32.h>

std::string KEY_PATH_RUN = "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";
std::string KEY_NAME_RUN = "limit-nvpstate";

std::string getBasePath(std::string path) {
    // find last occurrence of backslash
    size_t backslashPos = path.find_last_of("\\/");

    if (backslashPos == std::string::npos) {
        // no backslash found
        return "";
    }

    return path.substr(0, backslashPos);
}

std::string getProgramPath() {
    char path[MAX_PATH];

    if (GetModuleFileNameA(NULL, path, MAX_PATH) != 0) {
        return std::string(path);
    } else {
        return "";
    }
}

bool isAddedToStartup() {
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, KEY_PATH_RUN.c_str(), 0, KEY_READ, &hKey) != 0) {
        QMessageBox::critical(nullptr, "limit-nvpstate", "Error: Failed to open key");
        exit(1);
    }

    DWORD dataType;
    int result = RegQueryValueExA(hKey, KEY_NAME_RUN.c_str(), nullptr, &dataType, nullptr, nullptr);

    RegCloseKey(hKey);

    return result == 0;
}

int addToStartup(bool isEnabled) {
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, KEY_PATH_RUN.c_str(), 0, KEY_SET_VALUE, &hKey) != 0) {
        QMessageBox::critical(nullptr, "limit-nvpstate", "Error: Failed to open key");
        exit(1);
    }

    std::string programPath = getProgramPath();

    int result;
    if (isEnabled) {
        result = RegSetValueExA(hKey, KEY_NAME_RUN.c_str(), 0, REG_SZ, (const BYTE*)programPath.c_str(), programPath.size() + 1);
    } else {
        result = RegDeleteValueA(hKey, KEY_NAME_RUN.c_str());
    }

    RegCloseKey(hKey);

    return result;
}

std::string toLower(std::string str) {
    std::string lowerString = str;

    std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return lowerString;
}

std::string getBaseName(std::string& fullPath) {
    LPCSTR path = fullPath.c_str();
    LPCSTR executableName = PathFindFileNameA(path);
    return std::string(executableName);
}

std::string wStringToString(const std::wstring& wstr) {
    // get required buffer size for the multi-byte str, excluding the null terminator
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

    if (requiredSize == 0) {
        return "";
    }

    // create string with the correct size, excluding the null terminator
    std::string str(requiredSize - 1, '\0'); // subtract 1 to exclude the null terminator
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], requiredSize, NULL, NULL);
    return str;
}

std::string getProcessNameByPID(DWORD pid) {
    // attempt to get process name with OpenProcess
    // this will fail for protected processes
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (hProcess) {
        char executableName[MAX_PATH];
        int result = GetModuleBaseNameA(hProcess, nullptr, executableName, MAX_PATH);
        CloseHandle(hProcess);

        if (result > 0) {
            return executableName;
        }
    }

    // fallback for protected processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!hSnapshot) {
        return "";
    }

    std::string processName = "";

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &processEntry)) {
        do {
            if (processEntry.th32ProcessID == pid) {
                processName = wStringToString(processEntry.szExeFile);
                break;
            }
        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return processName;
}