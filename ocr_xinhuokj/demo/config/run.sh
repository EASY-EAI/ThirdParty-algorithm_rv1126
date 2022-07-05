#!/bin/sh

set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
cd $SHELL_FOLDER

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${SHELL_FOLDER}/lib




./demo ./1152-648.jpg  /oem/spen.key ./zh9k.dat 
