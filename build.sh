#!/bin/sh

set -e

CC=gcc
CCFLAGS=" -Wall -O0 -g -fsanitize=address,undefined"
LDFLAGS=" -fsanitize=address,undefined"

BUILD_DIR="build"
BIN_DIR="$BUILD_DIR/bin"
DEPFILE="$BUILD_DIR/.deps"

mkdir -p $BUILD_DIR
mkdir -p $BIN_DIR
touch $DEPFILE

TARGET=./pamde

INSTALL=false
RUN=true

objects=""
for file in src/c/*.c
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

if $INSTALL;
then
	sudo rm -fr /usr/share/pamde/pmd/
	sudo mkdir usr/share/pamde/pmd
	sudo cp src/pmd/* /usr/share/pamde/pmd
fi
