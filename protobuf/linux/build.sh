#!/bin/bash

./autogen.sh  # 生成 configure 文件
./configure --prefix=/home/bronze/housekeeper/third_party/build/protobuf
make
make check
sudo make install
sudo ldconfig
