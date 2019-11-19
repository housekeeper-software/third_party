make clean

echo "build armv7"
rm -rf openssl-1.0.2j
tar zxvf openssl-1.0.2j.tar.gz
source ./Setenv-android-armv7.sh
cd openssl-1.0.2j
./config shared no-asm no-ssl2 no-ssl3 no-comp no-hw no-engine --openssldir=/home/bronze/Documents/android/build/openssl/armv7 --prefix=/home/bronze/Documents/android/build/openssl/armv7
make depend && make all && make install

echo "show armv7 arch"
readelf -h ../build/openssl/armv7/lib/libcrypto.a | grep -i 'class\|machine' | head -2

echo "done"
