#!/bin/sh

SOURCE_DIR=`pwd`
BUILD_DIR='./build'
BUILD_TYPE='release'

ARGS=`getopt -o b: --long build-type: -- "$@"`
eval set -- "$ARGS"

while true ; do
    case "$1" in
        -b|--build-type)
            case "$2" in
                'debug'|'d') BUILD_TYPE='debug'; echo "build ${BUILD_TYPE} version."; shift 2 ;;
                'release'|'r')  BUILD_TYPE='release' ; echo "build ${BUILD_TYPE} version."; shift 2 ;;
            esac ;;
        --) shift ; break ;;
    esac
done

mkdir -p $BUILD_DIR/$BUILD_TYPE/lib \
    && mkdir -p $BUILD_DIR/$BUILD_TYPE/bin \
    && cd $BUILD_DIR/$BUILD_TYPE \
    && cmake \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE\
            $SOURCE_DIR \
    && make