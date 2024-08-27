#include <iostream>
#include <nvapi.h>
#include <Windows.h>
#include <unordered_set>
#include <Psapi.h>
#include <tlhelp32.h>
#include <fstream> 
#include <nlohmann/json.hpp>
#include <args.hxx>

typedef void* (*QueryPtr)(uint32_t);

HMODULE nvapi_handle;
std::unordered_set<std::string> process_exceptions;
bool is_pstate_unlimited = false;
NvPhysicalGpuHandle gpu_handles[NVAPI_MAX_PHYSICAL_GPUS];
nlohmann::json config;

std::string GetDirectoryPath(std::string path) {
    // find last occurrence of backslash
    size_t pos = path.find_last_of("\\/");

    if (pos == std::string::npos) {
        // return if no backslash is found
        return "";
    }

    return path.substr(0, pos);
}

std::string GetExecutablePath() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) != 0) {
        return std::string(path);
    } else {
        return "";
    }
}

bool IsAdmin() {
    bool admin = false;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
            admin = elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }

    return admin;
}

void* NvAPI_Query(uint32_t id) {
    return reinterpret_cast<QueryPtr>(GetProcAddress(nvapi_handle, "nvapi_QueryInterface"))(id);
}

int NvAPI_SetPstateClientLimits(NvPhysicalGpuHandle handle, unsigned int pstate_type, unsigned int pstate_limit) {
    static NvAPI_Status(*pointer)(NvPhysicalGpuHandle, unsigned int, unsigned int) = NULL;

    if (!pointer) {
        pointer = (NvAPI_Status(*)(NvPhysicalGpuHandle, unsigned int, unsigned int))NvAPI_Query(0xFDFC7D49);
    }

    if (!pointer) {
        return 1;
    }

    return (*pointer)(handle, pstate_type, pstate_limit);
}

std::string WStringToString(const std::wstring& wstr) {
    // get required buffer size for the multi-byte str
    int size_required = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

    if (size_required == 0) {
        return "";
    }

    // allocate buffer and perform the conversion
    std::string str(size_required, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_required, NULL, NULL);
    return str;
}

std::string GetProcessNameByPID(DWORD pid) {
    // attempt to get process name with OpenProcess
    // this will fail for protected processes
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (process_handle) {
        char executable_name[MAX_PATH];
        int result = GetModuleBaseNameA(process_handle, nullptr, executable_name, MAX_PATH);
        CloseHandle(process_handle);

        if (result > 0) {
            return executable_name;
        }
    }

    // fallback for protected processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!snapshot) {
        return "";
    }

    std::string process_name = "";

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &process_entry)) {
        do {
            if (process_entry.th32ProcessID == pid) {
                process_name = WStringToString(process_entry.szExeFile);
                break;
            }
        } while (Process32Next(snapshot, &process_entry));
    }

    CloseHandle(snapshot);
    return process_name;
}

void UnlimitPState(bool is_unlimit) {
    if (is_unlimit && !is_pstate_unlimited) {
        if (NvAPI_SetPstateClientLimits(gpu_handles[config["gpu_index"]], 3, 0) != 0) {
            std::cerr << "error: NvAPI_SetPstateClientLimits failed\n";
        }
        is_pstate_unlimited = true;
    } else if (!is_unlimit && is_pstate_unlimited) {
        if (NvAPI_SetPstateClientLimits(gpu_handles[config["gpu_index"]], 3, config["pstate_limit"]) != 0) {
            std::cerr << "error: NvAPI_SetPstateClientLimits failed\n";
        }
        is_pstate_unlimited = false;
    }
}

std::string ToLowerCase(std::string str) {
    std::string lower_string = str;

    std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return lower_string;
}

void CALLBACK WinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    // this corrects the hwnd after a EVENT_SYSTEM_FOREGROUND event
    hwnd = GetForegroundWindow();

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    std::string process_name = ToLowerCase(GetProcessNameByPID(pid));

    UnlimitPState(process_exceptions.count(process_name));
}

int main(int argc, char** argv) {
    if (!IsAdmin()) {
        std::cerr << "error: administrator privileges required\n";
        return 1;
    }

    std::string executable_directory = GetDirectoryPath(GetExecutablePath());
    std::wstring wexecutable_directory(executable_directory.begin(), executable_directory.end());

    if (SetCurrentDirectory(wexecutable_directory.c_str()) == 0) {
        std::cerr << "error: failed to change current directory\n";
        return 1;
    }

    std::string version = "4.0.0";

    args::ArgumentParser args_parser("limit-nvpstate Version " + version + " - GPLv3\nGitHub - https://github.com/valleyofdoom\n");
    args::HelpFlag args_help(args_parser, "", "display this help menu", { "help" });
    args::Flag args_gpu_indexes(args_parser, "", "display indexes for installed GPUs in the system", { "gpu-indexes" });
    args::Flag args_no_console(args_parser, "", "hide the console window", { "no-console" });

    try {
        args_parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << args_parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what();
        std::cerr << args_parser;
        return 1;
    } catch (args::ValidationError e) {
        std::cerr << e.what();
        std::cerr << args_parser;
        return 1;
    }

    if (args_no_console) {
        FreeConsole();
    }

    // load config
    std::ifstream config_json("config.json");

    if (!config_json.is_open()) {
        std::cerr << "error: failed to open config.json\n";
        return 1;
    }

    config_json >> config;

    // init nvapi
    if (NvAPI_Initialize() != 0) {
        std::cerr << "error: failed to initialize nvapi\n";
        return 1;
    }

    nvapi_handle = LoadLibraryA("nvapi64.dll");
    if (!nvapi_handle) {
        std::cerr << "error: failed loading nvapi64.dll\n";
        return 1;
    }

    unsigned long gpu_count = 0;
    if (NvAPI_EnumPhysicalGPUs(gpu_handles, &gpu_count) != 0) {
        std::cerr << "error: NvAPI_EnumPhysicalGPUs failed\n";
        return 1;
    }

    if (gpu_count == 0) {
        std::cerr << "error: 0 GPUs found";
        return 1;
    }

    if (args_gpu_indexes) {
        for (unsigned int i = 0; i < gpu_count; ++i) {
            char gpu_name[NVAPI_SHORT_STRING_MAX];

            if (NvAPI_GPU_GetFullName(gpu_handles[i], gpu_name) != 0) {
                std::cerr << "error: NvAPI_GPU_GetFullName failed\n";
                return 1;
            }

            std::cout << "GPU " << i << ": " << gpu_name << "\n";
        }
        return 0;
    }

    // gather process list
    for (std::string process_exception : config["process_exceptions"]) {
        process_exceptions.insert(ToLowerCase(process_exception));
    }

    // validate the specified GPU index
    bool is_index_valid = config["gpu_index"] >= 0 && config["gpu_index"] <= (gpu_count - 1);

    if (!is_index_valid) {
        std::cerr << "error: GPU index out of bounds. see indexes with --gpu-indexes\n";
        return 1;
    }

    // limit p-state
    if (NvAPI_SetPstateClientLimits(gpu_handles[config["gpu_index"]], 3, config["pstate_limit"]) != 0) {
        std::cerr << "error: NvAPI_SetPstateClientLimits failed\n";
        return 1;
    }

    HWINEVENTHOOK event_hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

    if (!event_hook) {
        std::cerr << "error: SetWinEventHook failed\n";
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWinEvent(event_hook);
    return 0;
}
