#!/bin/sh

set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
cd $SHELL_FOLDER

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${SHELL_FOLDER}/lib




./arcsoft_face_engine_test
