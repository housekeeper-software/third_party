#!/bin/sh
#export OPENSSL_ROOT_DIR=/home/bronze/Documents/android/build/openssl/arm64-v8a

#export LDFLAGS="-L${OPENSSL_ROOT_DIR}/lib"

./configure --host=aarch64-linux-android \
	CC="${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-gcc \
	--sysroot=${NDKROOT}/platforms/android-21/arch-arm64" \
	CFLAGS='-march=armv8-a -DANDROID -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__ -I${OPENSSL_ROOT_DIR}/include'  \
	OPENSSL_CFLAGS=-I${OPENSSL_ROOT_DIR}/include \
	OPENSSL_LIBS="-L${OPENSSL_ROOT_DIR}/lib -lssl -lcrypto" \
	--enable-openssl \
	--enable-shared \
	--enable-static  \
	--disable-libevent-regress \
	--disable-debug-mode \
	--disable-samples \
	--prefix=/home/bronze/Documents/android/build/libevent/arm64-v8a

#make && make install

