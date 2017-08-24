# 3D Modeling of a Scene with an Autonomous Robot

### Abstract
In today’s technology, the popularity of the robotic systems is getting increased due to the fact that they facilitate Daily life and that they are becoming more functionality. In linewith it, the robots that are cheap and easy to obtain are getting crucial. In the current study, a robot was created using materials cheap and easy to provide. After that, an autonomous navigation algorithm was designed and a 3D modeling system was formed with KinectFusion algorithm using an ASUS Xtion camera, which is able to give a rapid depth map, on a graphic card with an embedded NVIDIA Jetson TK1 having a rapid graphic processor. Then, this was integrated on the robot. In this way, it was aimed to create an autonomous robot being able to give a 3 dimensional model of the scene by moving autonomously and without striking the obstacle around.

If you can find more following paper:

http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7495911&isnumber=7495673

```
S. F. Pür, M. Kasap and Ö. Yılmaz, "3D modeling of a scene with an autonomous robot," 2016 24th Signal Processing and Communication Application Conference (SIU), Zonguldak, 2016, pp. 1001-1004.
doi: 10.1109/SIU.2016.7495911
```

Bibtex entry:
```
@INPROCEEDINGS {7495911,
    author = { S.F.Pür and M.Kasap and Ö.Yılmaz},
    booktitle = {2016 24 th Signal Processing and Communication Application Conference(SIU)},
    title = {3 D modeling of a scene with an autonomous robot},
    year = {2016},
    pages = {1001 - 1004},
    keywords = {control engineering computing;embedded systems;image sensors;path planning;robots;solid modelling;3 D modeling system;3 D scene modeling;ASUS Xtion camera;KinectFusion algorithm;autonomous navigation algorithm;autonomous robot;embedded NVIDIA Jetson TK1;graphic card;rapid depth map;robotic systems;Cameras;Navigation;Robot vision systems;Three - dimensional displays;Active Camera;Autonomous Navigation;Three Dimensional Modeling},
    doi = {10.1109 / SIU .2016 .7495911},
    month = {May},
}
```

# Installation

### Dependencies

#### Required

* TooN 2.2 : maths library.
* CMake 2.8+ : building tool.
* OpenCV 3.1 : computer vision library

##### Install TooN and CMake

```
#!shell
git clone git://github.com/edrosten/TooN.git
cd TooN
./configure
sudo make install
sudo apt-get install cmake

```
+(with Ubuntu, you might need to install the  build-essential package using ```sudo apt-get update && sudo apt-get install build-essential```)
 

#### Optional

* OpenMP : for the OpenMP version
* CUDA : for the CUDA version
* OpenCL : for the OpenCL version (OpenCL 1.1 or greater)

* OpenGL / GLUT : used by the graphical interface
* OpenNI : for the live mode, and for `oni2raw` tool 
* Freenect Drivers : In order to use the live mode.
* PkgConfig / Qt5 (using OpenGL) : used by the Qt graphical interface (not fully required to get a graphical interface)
* Python (numpy) : use by benchmarking scripts (`mean`, `max`, `min` functions)

##### Installation of Qt5 with an ARM board (ie. Arndale, ODROID,...)

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

##### Install OpenCV

```
[compiler] sudo apt-get install build-essential
[required] sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
[optional] sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
```

```
cd ~/<my_working_directory>
git clone https://github.com/Itseez/opencv.git
git clone https://github.com/Itseez/opencv_contrib.git
```

```
cd ~/opencv
mkdir build
cd build
```

```
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j7 # runs 7 jobs in parallel
sudo make install
```
### Compilation of Project

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


### Test Videos ###

https://www.youtube.com/playlist?list=PLypuuLR3edvwKqh6rEV9eOwGTwQGu1oQ9

### Note ###
This project was adapted from SLAMBench Project. Please, visit https://github.com/pamela-project/slambench for more information.
