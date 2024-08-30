#pragma once

#include <iostream>
#include <nvapi.h>

extern bool isPStateUnlimited;

int initNvAPI();
int SetPState(NvPhysicalGpuHandle hGPU, bool isUnlimit, unsigned int pStateLimit = 0);