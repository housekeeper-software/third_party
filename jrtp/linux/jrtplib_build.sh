#!/bin/bash

cmake 	-DCMAKE_CXX_FLAGS="-Wno-error=unused-result" \
	-DJTHREAD_INCLUDE_DIRS="/home/bronze/housekeeper/third_party/build/jrtp/include" \
	-DJTHREAD_LIBRARIES="/home/bronze/housekeeper/third_party/build/jrtp/lib" \
	-DCMAKE_INSTALL_PREFIX="/home/bronze/housekeeper/third_party/build/jrtp" \
	-DCMAKE_FIND_ROOT_PATH=/home/bronze/housekeeper/third_party/build/jrtp \
