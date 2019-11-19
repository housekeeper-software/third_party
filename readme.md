### 第三方库编译

1.先编译 openssl,因为很多其他库都依赖这个库
```
libcurl,libevent,onvif,osip
```

2.编译ffmpeg
```
先安装或者编译 nasm 和 yasm到系统目录即可，后面的编译要用到
再编译x264
再编译 ffmpeg
```

3.编译lua
```
编译 lua 需要安装 readline
sudo apt-get install readline-dev
```

4.编译pjproject
```
pjsip中有三个组件是我们需要的:
pjnath：p2p穿透
webrtc:回音消除模块（clone from chromium)
libyuv:YUV图像格式转换库，(clone from google)

在linux下，编译过程用到alsa库，所以，先编译alsa-lib库到系统目录
```

5.编译libcurl
```
编译之前，需要先编译c-ares,openssl
c-ares作为异步DNS解析库可以被libcurl使用
```
