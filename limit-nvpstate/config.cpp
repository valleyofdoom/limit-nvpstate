#include <fstream>
#include <nlohmann/json.hpp>
#include <QMessageBox>

nlohmann::json config;

std::string CONFIG_MAIN = "config.json";

int initConfig() {
    std::ifstream configMain(CONFIG_MAIN);

    if (!configMain.is_open()) {
        return 1;
    }

    configMain >> config;
    configMain.close();

    return 0;
}

void saveConfig() {
    std::ofstream configMain(CONFIG_MAIN);

    if (configMain.is_open()) {
        configMain << config.dump(4);
        configMain.close();
    } else {
        QMessageBox::critical(nullptr, "limit-nvpstate", "Error: Failed to save config");
        exit(1);
    }
}