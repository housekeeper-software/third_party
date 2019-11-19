#!/bin/bash
echo "build arm"
rm -rf openssl-1.0.2j
tar zxvf openssl-1.0.2j.tar.gz

source ./Setenv-android-arm.sh
cd openssl-1.0.2j
./config shared no-ssl2 no-ssl3 no-comp no-hw no-engine no-asm --openssldir=/home/bronze/Documents/android/build/openssl/arm --prefix=/home/bronze/Documents/android/build/openssl/arm
make depend && make all && make install


echo "show arm arch"
cd ../
readelf -h ../build/openssl/arm/lib/libcrypto.a | grep -i 'class\|machine' | head -2

echo "done"
