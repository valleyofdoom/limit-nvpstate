#pragma once

#include <iostream>
#include <nvapi.h>

extern bool isPStateUnlimited;

int initNvAPI();
int setPState(NvPhysicalGpuHandle hPhysicalGpu, bool isUnlimit, unsigned int pStateLimit = 0);