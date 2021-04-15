# LabAnalyser
A plugin based open source data modification and visualization tool

## How to compile LabAnalyser (inclomplete)

### Prerequisites

(tested on Arch Linux)
The following libraries are necessary:
- boost_1_59_0 (https://www.boost.org/)
- matOut (https://github.com/EyNuel/matOut/) to use MatOut please use the git patch in folder build-patches:
- hdf5-1.10.5 (https://github.com/HDFGroup/hdf5/)
- HighFive (https://github.com/BlueBrain/HighFive/)

### Steps  
(tested on Arch Linux)

1. install boost-libs 
   - Arch Linux: `pacman -S boost-libs`
2. create folder build/libs/
3. `cd build/libs`
4. `clone https://github.com/EyNuel/matOut.git` 
5. `cd matOut`
6. `patch < ../../../build-patches/MatOut-0001-Changes-to-use-the-lib-in-LabAnalyser.patch`
7. cd ../../../
8. qmake
9.  make

