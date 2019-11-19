#!/bin/sh
export PREBUILT=${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt
export PLATFORM=${NDKROOT}/platforms/android-19/arch-arm
export PREFIX=/home/bronze/Documents/android/build/x264/arm
export STRIP=$PREBUILT/bin/arm-linux-androideabi-strip

./configure --host=arm-linux-androideabi \
--enable-static \
--enable-pic \
--disable-asm \
--disable-cli \
--enable-strip \
--cross-prefix=$PREBUILT/linux-x86_64/bin/arm-linux-androideabi- \
--prefix=$PREFIX \
--sysroot=$PLATFORM
