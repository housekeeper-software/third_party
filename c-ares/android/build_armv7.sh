./configure --host=arm-linux-androideabi CC="${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc --sysroot=${NDKROOT}/platforms/android-19/arch-arm" --enable-shared --enable-static CFLAGS='-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16' --prefix=/home/bronze/Documents/android/build/c-ares/armv7a
