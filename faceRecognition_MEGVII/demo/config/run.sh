#!/bin/sh

set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
cd $SHELL_FOLDER

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${SHELL_FOLDER}


mkdir out_image

cd bin

sudo ./time_test ../model.yaml  -r bgr -i ../image/ -o ../out_image/



