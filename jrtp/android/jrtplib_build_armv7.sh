#!/bin/bash

CMAKE_TOOLCHAIN=/home/bronze/Desktop/android-cmake-master
cmake 	-DCMAKE_TOOLCHAIN_FILE=/home/bronze/Desktop/android-cmake-master/android.toolchain.cmake \
	-DANDROID_NDK=${NDKROOT}  \
	-DCMAKE_BUILD_TYPE=Release  \
	-DANDROID_ABI="armeabi-v7a"  \
	-DANDROID_NATIVE_API_LEVEL="android-19" \
	-DANDROID_STL="gnustl_static" \
	-DCMAKE_CXX_FLAGS="-Wno-error=unused-result" \
	-DJTHREAD_INCLUDE_DIRS="home/bronze/Documents/android/build/jrtp/armeabi-v7a/include" \
	-DJTHREAD_LIBRARIES="home/bronze/Documents/android/build/jrtp/armeabi-v7a/lib" \
	-DCMAKE_INSTALL_PREFIX="/home/bronze/Documents/android/build/jrtp/armeabi-v7a" \
	-DCMAKE_FIND_ROOT_PATH=/home/bronze/Documents/android/build/jrtp/armeabi-v7a \
	.

