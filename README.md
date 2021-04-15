# LabAnalyser
A plugin based open source data modification and visualization tool

## How to compile LabAnalyser (inclomplete)

### Prerequisites

(tested on Arch Linux)

- boost-libs
- matOut

### Steps

1. install boost-libs
2. create folder build/libs/
3. `cd build/libs`
4. `clone https://github.com/EyNuel/matOut.git` 
5. cd ../../
6. qmake
7. make
The following libraries are necessary:
1. boost_1_59_0 (https://www.boost.org/)
2. matOut (https://github.com/EyNuel/matOut/)
	to use MatOut please use the git patch in folder build-patches:
3. hdf5-1.10.5 (https://github.com/HDFGroup/hdf5/)
4. HighFive (https://github.com/BlueBrain/HighFive/)

