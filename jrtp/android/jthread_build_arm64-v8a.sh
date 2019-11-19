#!/bin/bash

CMAKE_TOOLCHAIN=/home/bronze/Desktop/android-cmake-master
cmake 	-DCMAKE_TOOLCHAIN_FILE=/home/bronze/Desktop/android-cmake-master/android.toolchain.cmake \
	-DANDROID_NDK=${NDKROOT}  \
	-DCMAKE_BUILD_TYPE=Release  \
	-DANDROID_ABI="arm64-v8a"  \
	-DANDROID_NATIVE_API_LEVEL="android-21" \
	-DANDROID_STL="gnustl_static" \
	-DCMAKE_INSTALL_PREFIX="/home/bronze/Documents/android/build/jrtp/arm64-v8a" \
	.

