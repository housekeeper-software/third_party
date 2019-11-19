#!/bin/sh
export PREBUILT=${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt
export PLATFORM=${NDKROOT}/platforms/android-19/arch-arm
export PREFIX=/home/bronze/Documents/android/build/x264/armv7
export STRIP=$PREBUILT/bin/arm-linux-androideabi-strip

./configure --host=arm-linux-androideabi \
--enable-static \
--disable-gpac \
--enable-pic \
--enable-strip \
--extra-cflags="-march=armv7-a  -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__" \
--cross-prefix=$PREBUILT/linux-x86_64/bin/arm-linux-androideabi- \
--prefix=$PREFIX \
--sysroot=$PLATFORM
