#!/bin/bash -e

./build-so.sh $NDKROOT \
                /home/bronze/Documents/android/openssl/openssl-1.0.2j \
                21 \
                arm64-v8a \
                4.9 \
                /home/bronze/Documents/android/build/openssl/arm64-v8a
