# limit-nvpstate

[![Downloads](https://img.shields.io/github/downloads/valleyofdoom/limit-nvpstate/total.svg)](https://github.com/valleyofdoom/limit-nvpstate/releases)

This program acts as a lightweight alternative to NVIDIA Inspector's "Multi-Display Power Saver" feature. The logic behind the program is to limit the GPU's P-State unless a process specified in the exceptions list is the foreground window (e.g. a video game), in which the P-State will be unlimited.

Place the program folder somewhere safe and enable "Add To Startup" in the File menu. Populate the process exclusion list with applications you do not wish P-States to be limited for while the application's window is in the foreground.

Note that closing the application by clicking "exit" in the tray menu will unlimit P-States before closing. If the program ends abruptly (ending the process in task manager), the unlimit action will not be executed when exiting.
