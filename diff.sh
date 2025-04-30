#!/bin/sh

set -e

print_help()
{
	echo "Usage: $0 [options]"
	echo "Options:"
	echo "  -r      Remove packages"
	echo "  -a      Add packages"
	echo "  -d      Only show diff"
	echo "  -d      Show common packages"
	echo "  -f      No confirm (Scary!)"
	echo "  -h      Display this help message"
}

if [ $# -eq 0 ]; then
    print_help
    exit 0
fi

f_remove=false
f_add=false
f_diff=false
f_common=false
f_force=false

while getopts "radcfh" opt; do
	case $opt in
		r)
			f_remove=true
			;;
		a)
			f_add=true
			;;
		d)
			f_diff=true
			;;
		c)
			f_common=true
			;;
		f)
			f_force=true
			;;
		h)
			print_help
			exit 0
			;;
		*)
			print_help
			exit 1
			;;
	esac
done

echo "Remove: $f_remove"
echo "Add:    $f_add"
echo "Diff:   $f_diff"
echo "Common:   $f_common"
echo "Force:  $f_force"

SOURCE="packages.list"

SOURCE_LIST=""
SYSTEM_LIST=""

check_directive() # line
{
	line="$1"
	case "$line" in
	!include*)
		line=$(sed 's/!include *//g' <<< "$line")
		line=$(sed 's/ *$//g' <<< "$line")
		parse_file "$line"
		return 0
		;;
	esac
	return 1
}

parse_file()
{
	file="$1"
	while IFS= read -r line || test -n "$line";
	do
		line=$(sed 's/#.*//' <<< "$line")  # remove comments
		line=$(sed 's/^ *//' <<< "$line")  # and leading whitespace

		if ! test -n "$line"; # skip empty
		then
			continue
		fi

		if ! check_directive "$line";
		then
			SOURCE_LIST="$SOURCE_LIST $line"
		fi
				
	done < "$file"
}

query_packages()
{
	SYSTEM_LIST=$(pacman -Qqe)
}

query_packages
parse_file "$SOURCE"
SOURCE_LIST=$(sed 's/^ *//' <<< "$SOURCE_LIST")
SOURCE_LIST=$(sed 's/ \+/\n/g' <<< "$SOURCE_LIST")

SOURCE_LIST=$(echo "$SOURCE_LIST" | sort)
SYSTEM_LIST=$(echo "$SYSTEM_LIST" | sort) # dont rely on pacman to sort the list

PKGS_NEW=$(comm -23 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))
PKGS_COM=$(comm -12 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))
PKGS_OLD=$(comm -13 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))

if $f_diff;
then
	if $f_remove;
	then
		echo remove:
		echo $PKGS_OLD
	fi
	if $f_add;
	then
		echo add:
		echo $PKGS_NEW
	fi
fi
	
