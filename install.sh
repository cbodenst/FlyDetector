#!/bin/bash

sudo cp fly-shaker.rules /etc/udev/rules.d/fly-shaker.rules
git pull
mkdir -p build
cd build
qmake ../FlyDetector.pro -r -spec linux-g++
make
mkdir -p ../bin
cp FlyDetector ../bin/FlyDetector

