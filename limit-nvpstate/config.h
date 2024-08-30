#pragma once

#include <nlohmann/json.hpp>

extern nlohmann::json config;

int initConfig();
void saveConfig();