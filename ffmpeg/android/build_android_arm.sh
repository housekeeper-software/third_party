#!/bin/bash 
SYSROOT=${NDKROOT}/platforms/android-19/arch-arm/
TOOLCHAIN=${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
X264=/home/bronze/Documents/android/build/x264/arm

function build_one { 
./configure \
     --prefix=$PREFIX \
     --enable-shared \
     --disable-static \
     --disable-doc \
     --disable-avdevice \
     --disable-doc \
     --disable-symver \
	--enable-pthreads \
	--enable-gpl \
	--enable-libx264 \
	--enable-encoder=libx264 \
     --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
     --target-os=linux \
     --arch=arm \
     --enable-cross-compile \
     --sysroot=$SYSROOT \
     --extra-cflags="-I$X264/include -Os -fpic $ADDI_CFLAGS" \
     --extra-ldflags="$ADDI_LDFLAGS -L$X264/lib" \
     $ADDITIONAL_CONFIGURE_FLAG 
     make clean 
     make 
     make install
 }
 CPU=arm
 PREFIX=/home/bronze/Documents/android/build/ffmpeg/$CPU
 ADDI_CFLAGS="-marm" 
 build_one
