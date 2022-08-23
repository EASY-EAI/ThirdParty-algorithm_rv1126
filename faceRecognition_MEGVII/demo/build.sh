#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status.
BuildDir=build

echo "building demo"
if [ ! -d "$BuildDir" ]; then
  # Take action if $BuildDir exists. #
  echo "create ${BuildDir}..."
  mkdir -p ${BuildDir}
fi

if [  "$1" =  "clear" ];
then
        rm -rf build;
        rm -rf Release;
        rm -rf bin;
        exit 0;
fi



toolchain_dir=/opt/rv1126_rv1109_sdk/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/

cd ${BuildDir}
rm  -f CMakeCache.txt
export PATH="${toolchain_dir}/bin:$PATH"
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-linux-gnueabihf.toolchain.cmake ..
echo "make -j$(nproc)"
make -j$(nproc)
cd ..

readonly Board=./Release
readonly Bin=./bin
readonly Lib=./lib
readonly Model=./model
readonly Config=./model.yaml
readonly Script=./config/run.sh
readonly Thirdparty=./thirdparty

mkdir Release
mkdir Release/bin
echo "copy ${Bin} to ${Board}"
cp -r ${Bin}/* ${Board}/bin
echo "copy ${Lib}/*.so to ${Board}"
cp -r ${Lib}/*.so ${Board}
echo "copy ${Thirdparty} to ${Board}"
rsync -avr --exclude='include' --exclude='sqlite3' ${Thirdparty} ${Board}/
echo "copy ${Model} to ${Board}"
cp -r ${Model} ${Board}
echo "copy ${Config} to ${Board}"
cp  ${Config} ${Board}
echo "copy ${Script} to ${Board}"
cp  ${Script} ${Board}

cp -r ./image ${Board}
rm -rf bin
rm -rf build



