export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/bronze/housekeeper/third_party/build/openssl/lib

./configure --enable-shared --enable-static --with-ssl=/home/bronze/housekeeper/third_party/build/openssl --enable-rtsp  --disable-debug-mode --enable-ares=/home/bronze/housekeeper/third_party/build/c-ares --prefix=/home/bronze/housekeeper/third_party/build/curl
~                                     
