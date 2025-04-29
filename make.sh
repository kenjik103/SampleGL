#!/bin/bash

absPath=$(pwd)
buildname=app

mkdir -p build

cd build

g++ -I $absPath/include $absPath/src/*.{cpp,c} -o $buildname -l GL -l glfw

cd $absPath

