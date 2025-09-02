#################################################
## HungPNQ
## Build MP4V2 Libraries
#################################################

#!/bin/bash

HOST=arm-linux-gnueabihf
INSTALL_DIR=/home/hunqp/Downloads/libmp4v2

export CC=arm-anycloud_v7.3.0-linux-uclibcgnueabi-gcc
export CXX=arm-anycloud_v7.3.0-linux-uclibcgnueabi-g++

autoreconf -fi

./configure --host=$HOST --prefix=$INSTALL_DIR --disable-debug --enable-static --disable-shared

make -j$(nproc)

make install