./configure --host=aarch64-linux-android CC="${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-gcc --sysroot=${NDKROOT}/platforms/android-21/arch-arm64" CFLAGS='-march=armv8-a -D__ANDROID__ -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__'   --enable-shared --enable-static --enable-pthread=force --enable-semaphore --prefix=/home/bronze/Documents/android/build/exosip2/arm64-v8a