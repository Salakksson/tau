#!/bin/sh

INPUT_FILE=""
INPUT_OBJECT=""
DEPFILE=".build/.deps"
COMPILER="gcc"

print_usage()
{
	echo usage: todo
}

die()
{
	echo "error: $1" >&2
	print_usage >&2
	
	exit 1
}

file_seen=0
while [ "$#" -gt 0 ]
do
	case "$1" in
	-d)
		[ $# -lt 2 ] && die "missing argument after -d"
		shift
		DEPFILE="$1"
		;;
	-o)
		[ $# -lt 2 ] && die "missing argument after -o"
		shift
		INPUT_OBJECT="$1"
		;;
	--compiler)
		[ $# -lt 2 ] && die "missing argument after --compiler"
		shift
		COMPILER="$1"
		;;
	-*)
		die "unknown option: $1"
		;;
	*)
		if [ "$file_seen" -eq 1 ]
		then
			die "only one input file can be specified"
		fi
		INPUT_FILE="$1"
		file_seen=1
		;;
	esac
	shift
done

[ "$file_seen" -eq 1 ] || die "no input file provided"

mkdir -p "$(dirname $DEPFILE)" && touch "$DEPFILE"

function get_deps() # file
{
	$COMPILER -MM "$1" | sed 's/\\//g' | cut -d ':' -f 2
}

function store_deps() # file, deps
{
	file=$1
	deps=${@:2} # Vargs
	
	depline="$file::$deps"
	while read -r line; 
	do
		qfile=$(echo $line | sed 's|\(.*\)::.*|\1|')
		if [ $qfile = $file ];
		then
			found=0
			break
		fi
	done < $DEPFILE
	if [ $found -eq 0 ];
	then
		sed -i "s|$line|$depline|" $DEPFILE
	else
		echo $depline >> $DEPFILE
	fi
}

function find_deps() # file, object
{
	file=$1
	object=$2
	found=1
	if [ ! -f $DEPFILE ];
	then
		touch $DEPFILE
		deps=$(get_deps $file)
		store_deps $file $deps
		echo $deps
		return
	elif [ $file -nt "$object" ];
	then
		deps=$(get_deps $file)
		store_deps $file $deps
		echo $deps
		return
	fi
	# Find location of file in depfile
	while read -r line; 
	do
		qfile=$(echo $line | sed 's|\(.*\)::.*|\1|')
		if [ $qfile = $file ];
		then
			found=0
			break
		fi
	done < $DEPFILE
	if [ $found -eq 0 ];
	then
		echo $line | sed 's|.*::\(.*\)|\1|'
		return
	else
		deps=$(get_deps $file)
		store_deps $file $deps
		echo $deps
		return
	fi
}

find_deps $INPUT_FILE $INPUT_OBJECT
