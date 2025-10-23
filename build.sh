#################################################
## HungPNQ
## Build MP4V2 Libraries
#################################################

#!/bin/bash

HOST=arm-anycloud_v7.3.0-linux-uclibcgnueabi
INSTALL_DIR=$PWD/libmp4v2
CROSS_COMPILER=$HOME/SDK/AnyCloud39AV100_SDK_V1.10/tools/arm-anycloud-linux-uclibcgnueabi-v7.3.0/bin/arm-anycloud_v7.3.0-linux-uclibcgnueabi-

export CC=${CROSS_COMPILER}gcc
export CXX=${CROSS_COMPILER}g++
export AR=${CROSS_COMPILER}ar
export RANLIB=${CROSS_COMPILER}ranlib
export STRIP=${CROSS_COMPILER}strip

mkdir -p $INSTALL_DIR

autoreconf -fi

./configure --host=$HOST --prefix=$INSTALL_DIR --disable-debug --enable-static --disable-shared

make -j$(nproc)

make install
