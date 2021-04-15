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
2. create folder build/libs/
3. `cd build/libs`
4. `clone https://github.com/EyNuel/matOut.git` 
5. cd ../../
6. qmake
7. make

