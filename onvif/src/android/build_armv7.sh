#!/bin/bash

CMAKE_TOOLCHAIN=/home/bronze/Desktop/android-cmake-master
cmake 	-DCMAKE_TOOLCHAIN_FILE=/home/bronze/Desktop/android-cmake-master/android.toolchain.cmake \
	-DANDROID_NDK=${NDKROOT}  \
	-DCMAKE_BUILD_TYPE=Release  \
	-DANDROID_ABI="armeabi-v7a"  \
	-DANDROID_NATIVE_API_LEVEL="android-19" \
	-DANDROID_STL="gnustl_static" \
	-DCMAKE_INSTALL_PREFIX="/home/bronze/Documents/android/build/onvif/armeabi-v7a" \
	.

