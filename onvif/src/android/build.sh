#!/bin/bash

mkdir /home/bronze/Documents/android/build/onvif
mkdir /home/bronze/Documents/android/build/onvif/armeabi
mkdir /home/bronze/Documents/android/build/onvif/armeabi-v7a
mkdir /home/bronze/Documents/android/build/onvif/arm64-v8a

./clean.sh
./build_arm.sh
make
cp libonvif.a /home/bronze/Documents/android/build/onvif/armeabi/libonvif.a

./clean.sh
./build_armv7.sh
make
cp libonvif.a /home/bronze/Documents/android/build/onvif/armeabi-v7a/libonvif.a

./clean.sh
./build_arm64-v8a.sh
make
cp libonvif.a /home/bronze/Documents/android/build/onvif/arm64-v8a/libonvif.a

echo "done"


