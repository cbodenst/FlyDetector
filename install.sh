#!/bin/bash

git pull
sudo cp fly-shaker.rules /etc/udev/rules.d/fly-shaker.rules
mkdir -p build
cd build
qmake-qt5 ../FlyDetector.pro -r -spec linux-g++
make
mkdir -p ../bin
cp FlyDetector ../bin/FlyDetector

