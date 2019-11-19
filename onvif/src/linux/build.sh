#!/bin/bash

cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=ON	-DCMAKE_INSTALL_PREFIX="/home/bronze/housekeeper/third_party/build/onvif" \
	.

