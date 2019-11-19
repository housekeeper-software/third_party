#!/bin/sh
export PREBUILT=${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt
export PLATFORM=${NDKROOT}/platforms/android-21/arch-arm64
export PREFIX=/home/bronze/Documents/android/build/x264/arm64-v8a
#export STRIP=$PREBUILT/bin/aarch64-linux-android-strip

./configure --host=aarch64-linux-android \
--enable-static \
--disable-gpac \
--enable-pic \
--enable-strip \
--extra-cflags="-march=armv8-a -D__ANDROID__ -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__" \
--cross-prefix=$PREBUILT/linux-x86_64/bin/aarch64-linux-android- \
--prefix=$PREFIX \
--sysroot=$PLATFORM
