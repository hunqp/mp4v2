#################################################
## HungPNQ
## Build MP4V2 Libraries
#################################################

#!/bin/bash

HOST=arm-anycloud_v7.3.0-linux-uclibcgnueabi
INSTALL_DIR=$PWD/libmp4v2

export CC=arm-anycloud_v7.3.0-linux-uclibcgnueabi-gcc
export CXX=arm-anycloud_v7.3.0-linux-uclibcgnueabi-g++
export AR=arm-anycloud_v7.3.0-linux-uclibcgnueabi-ar
export RANLIB=arm-anycloud_v7.3.0-linux-uclibcgnueabi-ranlib
export STRIP=arm-anycloud_v7.3.0-linux-uclibcgnueabi-strip

mkdir -p $INSTALL_DIR

autoreconf -fi

./configure --host=$HOST --prefix=$INSTALL_DIR --disable-debug --enable-static --disable-shared

make -j$(nproc)

make install
