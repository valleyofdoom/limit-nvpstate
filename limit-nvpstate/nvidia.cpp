#include <iostream>
#include <nvapi.h>
#include <Windows.h>

typedef void* (*QueryPtr)(uint32_t);

HMODULE hNvAPI;
std::string NVAPI_DLL = "nvapi64.dll";
bool isPStateUnlimited = true;

int initNvAPI() {
    if (NvAPI_Initialize() != 0) {
        std::cerr << "error: failed to initialize nvapi\n";
        return 1;
    }

    hNvAPI = LoadLibraryA(NVAPI_DLL.c_str());

    if (!hNvAPI) {
        std::cerr << "error: failed loading nvapi64 dll\n";
        return 1;
    }

    return 0;
}

void* NvAPI_Query(uint32_t id) {
    return reinterpret_cast<QueryPtr>(GetProcAddress(hNvAPI, "nvapi_QueryInterface"))(id);
}

int NvAPI_SetPstateClientLimits(NvPhysicalGpuHandle hPhysicalGpu, unsigned int pstateType, unsigned int pStateLimit) {
    static NvAPI_Status(*pointer)(NvPhysicalGpuHandle, unsigned int, unsigned int) = NULL;

    if (!pointer) {
        pointer = (NvAPI_Status(*)(NvPhysicalGpuHandle, unsigned int, unsigned int))NvAPI_Query(0xFDFC7D49);
    }

    if (!pointer) {
        return 1;
    }

    return (*pointer)(hPhysicalGpu, pstateType, pStateLimit);
}

int setPState(NvPhysicalGpuHandle hPhysicalGpu, bool isUnlimit, unsigned int pStateLimit = 0) {
    if (isUnlimit && !isPStateUnlimited) {
        if (NvAPI_SetPstateClientLimits(hPhysicalGpu, 3, 0) != 0) {
            std::cerr << "error: NvAPI_SetPstateClientLimits failed\n";
            return 1;
        }

        std::cout << "info: set P0\n";
        isPStateUnlimited = true;
    } else if (!isUnlimit && isPStateUnlimited) {
        if (NvAPI_SetPstateClientLimits(hPhysicalGpu, 3, pStateLimit) != 0) {
            std::cerr << "error: NvAPI_SetPstateClientLimits failed\n";
            return 1;
        }

        std::cout << "info: set P" << pStateLimit << "\n";
        isPStateUnlimited = false;
    }

    return 0;
}