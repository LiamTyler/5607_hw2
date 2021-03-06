# Ray Tracer

## Installation
From Zip file:
```sh
unzip 5607_hw2-master.zip
cd 5607_hw2-master/ext/nanogui
git clone --recursive https://github.com/wjakob/nanogui.git ./
cd ../../
mkdir build
cd build
cmake ..
make -j
```

## Usage
- Just run the executable 'RayTracer' from the command line. It will open a GUI where you can select which scene you want to load, and trace that scene. It will display the progress and finished scene.
- If you want to not use the GUI, just run './RayTracer <path to my scene>' and it will run the tracer on that scene silently

## Scene File
- The Scene file works as described in the sceneFileDescription.pdf, but with the addition that you can change the sampling method by adding the following line to you scene file:
sampling_method <number>   (where 0 = basic, 1 = Jittered, 2 = Adaptive)
