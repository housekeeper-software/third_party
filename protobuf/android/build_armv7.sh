#!/bin/bash

CMAKE_TOOLCHAIN=/home/bronze/Desktop/android-cmake-master
cmake 	-DCMAKE_TOOLCHAIN_FILE=/home/bronze/Desktop/android-cmake-master/android.toolchain.cmake \
	-DANDROID_NDK=${NDKROOT}  \
	-DCMAKE_BUILD_TYPE=Release  \
	-DANDROID_ABI="armeabi-v7a"  \
	-DANDROID_NATIVE_API_LEVEL="android-19" \
	-DCMAKE_CXX_FLAGS="-Wno-error=unused-result" \
	-DANDROID_STL="gnustl_static" \
    	-Dprotobuf_BUILD_TEST=OFF \
    	-Dprotobuf_BUILD_EXAMPLES=OFF \
  	-DCMAKE_VERBOSE_MAKEFILE=ON \
   	-Dprotobuf_BUILD_SHARED_LIBS=OFF \
    	-Dprotobuf_BUILD_STATIC_LIBS=ON \
	-DANDROID_LINKER_FLAGS="-landroid -llog" \
    	-DANDROID_CPP_FEATURES="rtti exceptions" \
	-DCMAKE_INSTALL_PREFIX="/home/bronze/Documents/android/build/protobuf/armeabi-v7a" \
	-DCMAKE_FIND_ROOT_PATH=/home/bronze/Documents/android/build/protobuf/armeabi-v7a \
	./cmake

