# README #
3D Modeling of a Scene with an Autonomous Robot

With the current study, an autonomous robots was produced at a reasonable cost compared to other industrial robots serving for the same purpose and also a 3 dimensional modeling system that was created using an algorithm was integrated on this robot. In doing so, it was aimed to create a 3 dimensional model of the scene where the autonomous robot was located.

# Installation #

### Dependencies ###

#### Required ####

* TooN 2.2 : maths library.
* CMake 2.8+ : building tool.

##### Install TooN and CMake#####

```
#!shell
git clone git://github.com/edrosten/TooN.git
cd TooN
./configure
sudo make install
sudo apt-get install cmake

```
+(with Ubuntu, you might need to install the  build-essential package using ```sudo apt-get update && sudo apt-get install build-essential```)
 

#### Optional ####

* OpenMP : for the OpenMP version
* CUDA : for the CUDA version
* OpenCL : for the OpenCL version (OpenCL 1.1 or greater)

* OpenGL / GLUT : used by the graphical interface
* OpenNI : for the live mode, and for `oni2raw` tool 
* Freenect Drivers : In order to use the live mode.
* PkgConfig / Qt5 (using OpenGL) : used by the Qt graphical interface (not fully required to get a graphical interface)
* Python (numpy) : use by benchmarking scripts (`mean`, `max`, `min` functions)

##### Installation of Qt5 with an ARM board (ie. Arndale, ODROID,...) #####

On ARM board, the default release of Qt5 was compile using OpenEGL, to use the Qt interface, you will have to compile Qt :

```
#!
cd ~
wget http://download.qt-project.org/official_releases/qt/5.2/5.2.1/single/qt-everywhere-opensource-src-5.2.1.tar.gz
tar xzf qt-everywhere-opensource-src-5.2.1.tar.gz
cd ~/qt-everywhere-opensource-src-5.2.1
./configure -prefix ~/.local/qt/ -no-compile-examples  -confirm-license  -release   -nomake tests   -nomake examples
make
make install
```

### Compilation of Project ###

Then simply build doing: 

```
#!
make
```

To use qt, if you compile the source as explained above, you should need to specify the Qt install dir :
```
#!
CMAKE_PREFIX_PATH=~/.local/qt/ make
```


##### Tests

https://www.youtube.com/playlist?list=PLypuuLR3edvwKqh6rEV9eOwGTwQGu1oQ9

##### Note
This project is adapted from SLAMBench Project. Please, visit https://github.com/pamela-project/slambench for more information
