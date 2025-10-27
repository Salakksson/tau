#!/bin/sh

CC=gcc
CCFLAGS=" -Wall -Wpedantic -Werror -O0 -g -fsanitize=address,undefined -fno-sanitize=leak"
LDFLAGS=" -fsanitize=address,undefined -fno-sanitize=leak"

BUILD_DIR="build"
BIN_DIR="$BUILD_DIR/bin"
DEPFILE="$BUILD_DIR/.deps"

mkdir -p $BUILD_DIR
mkdir -p $BIN_DIR
touch $DEPFILE

TARGET=./tauc

INSTALL=false
RUN=true

if [ "$1" = "clean" ];
then
	rm -fr $BIN_DIR/*/*.o
fi

objects=""
for file in src/*.c
do
	out=${file/src/$BIN_DIR}.o
	objects="$objects $out"

	skip=true

	for dep in $(build/fastdep.sh $file -o $out -d $DEPFILE)
	do
		if [ $file -nt $out ];
		then
			skip=false;
		fi
	done

	if $skip;
	then
		echo × skipping $file → $out
		continue
	fi

	echo $CC -c $CCFLAGS $file -o $out
	$CC -c $CCFLAGS $file -o $out
done

$CC $LDFLAGS $objects -o $TARGET

if $RUN;
then
	./$TARGET
fi
