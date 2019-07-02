#!/bin/sh

SOURCE_DIR=`pwd`
BUILD_DIR='./build'
BUILD_TYPE='debug'

mkdir -p $BUILD_DIR/$BUILD_TYPE/lib \
    && mkdir -p $BUILD_DIR/$BUILD_TYPE/bin \
    && cd $BUILD_DIR/$BUILD_TYPE \
    && cmake \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE\
            $SOURCE_DIR \
    && make