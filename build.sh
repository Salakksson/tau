#!/bin/sh

set -e

CC=gcc
CCFLAGS=" -Wall -O0 -g"
LDFLAGS=" "

BUILD_DIR="build"
BIN_DIR="$BUILD_DIR/bin"
DEPFILE="$BUILD_DIR/.deps"

mkdir -p $BUILD_DIR
mkdir -p $BIN_DIR
touch $DEPFILE

TARGET=./pamde

objects=""
for file in src/*.c
do
	out=${file/src/$BIN_DIR}.o
	objects="$objects $out"

	skip=false

	# for dep in $(.build/fastdep.sh $file -o $out -d $DEPFILE)
	# do
	#	if [ $file -nt $out ];
	#	then
	#		skip=false;
	#	fi
	# done

	if $skip;
	then
		echo × skipping $file → $out
		continue
	fi

	echo $CC -c $CCFLAGS $file -o $out
	$CC -c $CCFLAGS $file -o $out
done

$CC $LDFLAGS $objects -o $TARGET

