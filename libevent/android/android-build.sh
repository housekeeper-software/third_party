#!/bin/sh
#export OPENSSL_ROOT_DIR=/home/bronze/Documents/android/build/openssl/arm

#export LDFLAGS="-L${OPENSSL_ROOT_DIR}/lib"

./configure --host=arm-linux-androideabi \
	CC="${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc \
	--sysroot=${NDKROOT}/platforms/android-14/arch-arm" \
	CFLAGS='-mfloat-abi=softfp -mfpu=vfpv3-d16 -DANDROID -I${OPENSSL_ROOT_DIR}/include'  \
	OPENSSL_CFLAGS=-I${OPENSSL_ROOT_DIR}/include \
	OPENSSL_LIBS="-L${OPENSSL_ROOT_DIR}/lib -lssl -lcrypto" \
	--enable-openssl \
	--enable-shared \
	--enable-static  \
	--disable-libevent-regress \
	--disable-debug-mode \
	--prefix=/home/bronze/Documents/android/build/libevent/arm

#make && make install

