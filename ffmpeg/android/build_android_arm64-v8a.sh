#!/bin/bash 
SYSROOT=${NDKROOT}/platforms/android-21/arch-arm64/
TOOLCHAIN=${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
X264=/home/bronze/Documents/android/build/x264/arm64-v8a

function build_one { 
./configure \
     --prefix=$PREFIX \
     --enable-shared \
     --disable-static \
     --disable-doc \
     --disable-avdevice \
	--enable-pthreads \
     --disable-symver \
	--enable-gpl \
	--enable-libx264 \
	--enable-encoder=libx264 \
     --cross-prefix=$TOOLCHAIN/bin/aarch64-linux-android- \
     --target-os=linux \
     --arch=aarch64 \
     --cpu=armv8-a \
     --enable-neon \
     --enable-cross-compile \
     --enable-asm \
     --sysroot=$SYSROOT \
     --extra-cflags="-I$X264/include -Os -fpic $ADDI_CFLAGS" \
     --extra-ldflags="$ADDI_LDFLAGS -L$X264/lib" \
     $ADDITIONAL_CONFIGURE_FLAG 
     make clean 
     make 
     make install
 }
 CPU=arm64-v8a
 PREFIX=/home/bronze/Documents/android/build/ffmpeg/$CPU
 ADDI_CFLAGS="-march=armv8-a -D__ANDROID__ -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__" 
 build_one
