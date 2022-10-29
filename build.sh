#!/bin/bash

#CMAKE_BUILD_TYPE="Release"
#CMAKE_BUILD_TYPE="Debug"

DIR="./build/"
if [ -d "$DIR" ]; then
  # Take action if $DIR exists. #
  echo "building project in ${DIR}..."
else
  mkdir build
  echo "mkdir ${DIR} ... "
fi

echo "build dir: $DIR"; g++ -O3 -finline-functions Complete_analysis.cpp `root-config --cflags --glibs` -o build/main.exe -lSpectrum;
#cmake . -B${DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}; cd ${DIR}; make VERBOSE=1


