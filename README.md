**Table of Contents**

- [Functionality of LabAnalyser](#functionality-of-labanalyser)
  * [How to Compile LabAnalyser](#how-to-compile-labanalyser)
    + [For Windows](#for-windows)
    + [For Linux (Tested on Arch Linux)](#for-linux)
  * [Known Bugs](#known-bugs)

# Functionality of LabAnalyser

LabAnalyser is a plugin-based, open-source tool for data modification and visualization.

---

**Create editable variables (parameters) or data within a plugin (refer to https://github.com/ConverterLabs/PluginTemplate) and utilize LabAnalyser for visualization.
Design user interfaces with QTCreator, load them into LabAnalyser, and link the UI elements to variables using a drag-and-drop mechanism.**
![LabAnalyser](readme_pictures/show_variables.png)

---

**Employ the signal-slot system of Qt in QTCreator to develop sophisticated user interfaces.**

![Stateflow Visualization](readme_pictures/UseQTCreator.png)

---

**Load multiple user interfaces as required into LabAnalyser to visualize data points in real-time, scaling to hundreds of thousands.**
![Array of Windows on Four Screens](readme_pictures/UndockAndCreate_MonitorArray.png)

---

**Leverage features such as export to HDF5 or MATLAB files (*.mat) for data storage, or establish a direct connection to MATLAB via TCP/IP.**
![Data Export Options](readme_pictures/export.png)

---

# How to Compile LabAnalyser

## For Windows

Follow these steps using MSYS2, and install the necessary packages:

1. `pacman -Syuu` (run three times to ensure all packages are updated)
2. `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`
3. `pacman -S mingw-w64-x86_64-qt6`
4. `pacman -S mingw-w64-x86_64-qt-creator`
5. `pacman -S mingw-w64-x86_64-boost`
6. `pacman -S mingw-w64-x86_64-highfive`
7. `pacman -S mingw-w64-x86_64-fftw`
8. `pacman -S git`
9. Clone the repository: `git clone https://github.com/EyNuel/matOut.git`
10. Apply the patch: `patch < ../../../build-patches/MatOut-0001-Changes-to-use-the-lib-in-LabAnalyser.patch`
11. Open the MinGW-w64 32-Bit or 64-Bit Shell and launch `qtcreator`.
12. Open `LabAnalyser.pro`

### Finding All Dependencies

```bash
ldd ~/build64/* | grep -iv system32 | grep -vi windows | grep -v :$ | cut -f2 -d\> | cut -f1 -d\( | tr \\ / | while read a; do ! [ -e "build64/`basename $a`" ] && cp -v "$a" ~/build64/; done
```

## For Linux (Tested on Arch Linux)

1. Install Boost libraries:
   - `pacman -S boost-libs`
2. Install HighFive:
   - From AUR: `yay -S highfive` or `yaourt -S highfive`
3. Prepare the build environment:
   - `mkdir build && mkdir build/libs/`
   - `cd build/libs/`
   - `git clone https://github.com/EyNuel/matOut.git`
   - `cd matOut`
   - `patch < ../../../build-patches/MatOut-0001-Changes-to-use-the-lib-in-LabAnalyser.patch`
   - `cd ../../`
   - `qmake ../`
   - `make`
   - If successful, execute: `./LabAnalyser`

# Known Bugs

Changing the monitor array or the system configuration may cause LabAnalyser to crash due to a bug in `QMainWindow::restoreState` of Qt. To resolve this, open the corresponding .LAexp file with a text editor and remove the line just before the last one containing the window state `<State> .... </State>`.
