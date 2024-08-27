# limit-nvpstate

[![Downloads](https://img.shields.io/github/downloads/valleyofdoom/limit-nvpstate/total.svg)](https://github.com/valleyofdoom/limit-nvpstate/releases)

This program acts as a lightweight alternative to NVIDIA Inspector's "Multi-Display Power Saver" feature. The logic behind the program is to limit the GPU's P-State unless a process specified in the exceptions list is the foreground window (e.g. a video game), in which the P-State will be unlimited.

## Usage

### Config

To get started, open ``config.json`` in a text editor to edit the fields below.

- ``gpu_index`` - typically ``0`` in systems with a single GPU, but all the indexes can be obtained with the ``--gpu-indexes`` argument

- ``pstate_limit`` - the P-State limit for example,``8`` for P-State 8 (P8)

- ``process_exceptions`` - array of process name exceptions (e.g. ``notepad.exe``, ``mspaint.exe``)

### Launch at Startup

Place the program folder somewhere safe and use Task Scheduler ([instructions](https://www.windowscentral.com/how-create-automated-task-using-task-scheduler-windows-10)) with the following command (as an example) to have the program start at windows logon. Ensure to enable ``Run with highest privileges`` as the program requires administrator privileges.

```bat
C:\limit-nvpstate\limit-nvpstate.exe --no-console
```
