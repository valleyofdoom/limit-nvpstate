#include <limitnvpstate.h>
#include <utils.h>
#include <iostream>
#include <config.h>

int main(int argc, char* argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    CreateMutexA(NULL, 1, "limit-nvpstate");

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hWnd = FindWindowA(NULL, "limit-nvpstate");

        if (hWnd) {
            SetForegroundWindow(hWnd);
        }
        return 0;
    }

    // cd to the directory of the current executable
    std::string programExecutableDirectory = getBasePath(getProgramPath());
    std::filesystem::current_path(programExecutableDirectory);

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
