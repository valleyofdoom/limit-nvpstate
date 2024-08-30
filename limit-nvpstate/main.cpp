#include <limitnvpstate.h>
#include <utils.h>
#include <iostream>
#include <config.h>

int main(int argc, char* argv[]) {
    // cd to the directory of the current executable
    std::string executableDirectory = getBasePath(getProgramPath());
    std::filesystem::current_path(executableDirectory);

    // init config
    if (initConfig() != 0) {
        std::cerr << "error: failed to init config\n";
        return 1;
    }

    QApplication a(argc, argv);
    limitnvpstate w;

    if (config["start_minimized"]) {
        w.hide();
    } else {
        w.show();
    }

    return a.exec();
}
