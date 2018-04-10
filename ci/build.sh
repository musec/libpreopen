#!/bin/sh -ex

mode="$1"
if [ "${mode}" = "" ]
then
    echo "Usage:  $0 <Debug|Release>"
    exit 1
fi

mkdir -p build/${mode}
cd build/${mode}
nice cmake -G Ninja -D CMAKE_BUILD_TYPE=${mode} ../..
nice ninja -v
nice ninja check || true
