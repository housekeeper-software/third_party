cmake_minimum_required(VERSION 3.5)

project(onvif)
set(VERSION 1.0.0)
set(CMAKE_BUILD_TYPE "Release")

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/../)

include_directories(
	/home/bronze/housekeeper/third_party/build/openssl/include
	${SOURCE_DIR}/json
	${SOURCE_DIR}/src
)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_NO_C_LOCALE -DWITH_NONAMESPACES -DWITH_OPENSSL -D__STDC_FORMAT_MACROS -pthread -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_NO_C_LOCALE -DWITH_NONAMESPACES -DWITH_OPENSSL -D__STDC_FORMAT_MACROS  -DJSON_IS_AMALGAMATION -DJSONCPP_NO_LOCALE_SUPPORT -std=c++11 -frtti -fexceptions -pthread -Wall")

file(GLOB_RECURSE SRC ${SOURCE_DIR}/src/*.c*)
add_library(onvif STATIC ${SRC})

