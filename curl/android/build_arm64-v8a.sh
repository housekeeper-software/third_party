#export LDFLAGS="-L/home/bronze/Documents/android/build/c-ares/arm64-v8a/lib"

#export LIBS="-ldl -lcares"

./configure --host=aarch64-linux-android CC="${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-gcc --sysroot=${NDKROOT}/platforms/android-21/arch-arm64" CFLAGS='-march=armv8-a -D__ANDROID__ -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__'  --enable-shared --enable-static --with-ssl=/home/bronze/Documents/android/build/openssl/arm64-v8a --enable-rtsp  --disable-debug-mode --prefix=/home/bronze/Documents/android/build/curl/arm64-v8a
