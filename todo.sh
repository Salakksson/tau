#!/bin/bash

ALL=false

if [ "$1" = "all" ] ;
then
	ALL=true
fi

FIND="todo"

IGNORED="src/diag.c src/diag.h"

SPECS="src/"

for f in $IGNORED;
do
	spec=":!$f"
	SPECS="$SPECS $spec"
done

output=$(git grep -nE --ignore-case $FIND -- $SPECS)

lines=$(echo -e "$output" | wc -l)

echo -e "Found $lines TODOs"

if $ALL ;
then
	echo "$output"
	exit 0
fi
index=$(( $RANDOM % $lines + 1))

sed "${index}q;d" <(echo "$output")
