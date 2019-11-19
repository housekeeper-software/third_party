#!/bin/sh

./configure \
	--prefix=/home/bronze/housekeeper/third_party/build/libevent \
	CFLAGS="-I/home/bronze/housekeeper/third_party/build/openssl/include" \
	LDFLAGS="-L/home/bronze/housekeeper/third_party/build/openssl/lib -lssl -lcrypto" \
	--enable-openssl \
	--enable-shared \
	--enable-static  \
	--disable-libevent-regress \
	--disable-debug-mode \
	--disable-samples
	

