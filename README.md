# LabAnalyser
A plugin based open source data modification and visualization tool

## How to compile LabAnalyser

### Prerequisites

(tested on Arch Linux)


### Steps  
(tested on Arch Linux)

1. install boost-libs 
   - Arch Linux: `pacman -S boost-libs`
2. install HighFive
   - Arch Linux: install from AUR `yay -S highfive` or `yaourt -S highfive`
3. create folder build/libs/
4. `cd build/libs`
5. `clone https://github.com/EyNuel/matOut.git` 
6. `cd matOut`
7. `patch < ../../../build-patches/MatOut-0001-Changes-to-use-the-lib-in-LabAnalyser.patch`
8. cd ../../../
9.  qmake
10. make
11. if successful: execude LabAnalyser `./LabAnalyser`

