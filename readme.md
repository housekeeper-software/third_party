### 第三方库编译
先编译 openssl,因为很多其他库都依赖这个库
编译ffmpeg之前需要先编译 x264 ,
还需要编译nasm, yasm，直接将这两个库编译到系统目录即可
编译lua之前，需要安装readline
编译pjproject之前需要编译 alsa-lib