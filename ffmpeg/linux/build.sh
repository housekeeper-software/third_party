#!/bin/bash 
./configure \
	--prefix=/home/bronze/housekeeper/third_party/build/ffmpeg \
	--enable-shared \
	--disable-static \
	--disable-doc \
	--disable-avdevice \
	--enable-pthreads \
	--disable-symver \
	--enable-gpl \
	--enable-cross-compile \
	--enable-libx264 \
	--enable-encoder=libx264 \
	--enable-asm \
	--extra-cflags="-I/usr/local/include -I/home/bronze/housekeeper/third_party/build/x264/include -Os -fpic" \
	--extra-ldflags="-L/usr/local/lib -L/home/bronze/housekeeper/third_party/build/x264/lib"

