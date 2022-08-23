#!/bin/sh

set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
cd $SHELL_FOLDER

CUR_DIR_NAME=`basename "$SHELL_FOLDER"`

if [ "$1" = "clear" ]; then
	rm -rf build
	rm -rf Release
	exit 0
fi

rm -rf build
mkdir build
cd build
cmake ..
make -j24

mkdir -p "../Release" && cp arcsoft_face_engine_test  "../Release"
mkdir ../Release/images
mkdir ../Release/key

cp ../images/*.jpeg ../Release/images
cp ../images/*.NV21 ../Release/images
cp ../key/* ../Release/key
mkdir ../Release/lib
cp ../lib/* -d ../Release/lib

cp ../config/run.sh ../Release
