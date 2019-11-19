#!/bin/bash

CMAKE_TOOLCHAIN=/home/bronze/Desktop/android-cmake-master
cmake 	-DCMAKE_TOOLCHAIN_FILE=/home/bronze/Desktop/android-cmake-master/android.toolchain.cmake \
	-DANDROID_NDK=${NDKROOT}  \
	-DCMAKE_BUILD_TYPE=Release  \
	-DANDROID_ABI="armeabi"  \
	-DANDROID_NATIVE_API_LEVEL="android-19" \
	-DCMAKE_CXX_FLAGS="-DJSON_IS_AMALGAMATION -DJSONCPP_NO_LOCALE_SUPPORT -I../../include --sysroot=${NDKROOT}/platforms/android-19/arch-arm"
	-DANDROID_STL="gnustl_static" \
	-DCMAKE_INSTALL_PREFIX="/home/bronze/Documents/android/build/jsoncpp/arm" \
	.


