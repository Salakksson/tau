#!/bin/sh

set -e

print_help()
{
	echo "Usage: $0 [options]"
	echo "Options:"
	echo "  -r      Remove packages"
	echo "  -a      Add packages"
	echo "  -d      Only show diff"
	echo "  -c      Show common packages"
	echo "  -f      No confirm (Scary!)"
	echo "  -q      Quiet mode"
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
f_verbose=true

while getopts "radcfqh" opt; do
	case $opt in
		r) f_remove=true ;;
		a) f_add=true ;;
		d) f_diff=true ;;
		c) f_common=true ;;
		f) f_force=true ;;
		q) f_verbose=false ;;
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
SOURCE_LIST=$(sed 's/\s\+/\n/g' <<< "$SOURCE_LIST")
SYSTEM_LIST=$(sed 's/\s\+/\n/g' <<< "$SYSTEM_LIST")

SOURCE_LIST=$(echo "$SOURCE_LIST" | sort)
SYSTEM_LIST=$(echo "$SYSTEM_LIST" | sort) # dont rely on pacman to sort the list

PKGS_ADD=$(comm -23 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))
PKGS_REM=$(comm -13 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))
PKGS_COM=$(comm -12 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))

PKGS_ADD=$(echo $PKGS_ADD)
PKGS_REM=$(echo $PKGS_REM)
PKGS_COM=$(echo $PKGS_COM)

RESET='\033[0m'

BOLD='\033[1m'

FG_RED='\033[31m'
FG_GREEN='\033[32m'
FG_YELLOW='\033[33m'
FG_BLUE='\033[34m'
FG_MAGENTA='\033[35m'
FG_CYAN='\033[36m'
FG_WHITE='\033[97m'
FG_GRAY='\033[90m'

verbose()
{
	if $f_verbose;
	then
		echo -e $1
	fi
}

confirm() # command
{
	if $f_force;
	then
		$@
		return 0
	fi
	
	echo -e "$BOLD Are you sure you want to run: (y/N) $RESET"
	echo -e "$FG_BLUE $@ $RESET"
	read responce
	case $responce in
		[y/Y]*) $@ ;;
		*) return 1 ;;
	esac
}

remove_packages()
{
	confirm sudo pacman -Rs $PKGS_REM
}

add_packages()
{
	confirm sudo pacman -S $PKGS_ADD
}

print_diff()
{
	if $f_remove && test -n "$PKGS_REM";
	then
		verbose Remove:
		printf "${BOLD}${FG_RED}%s${RESET}\n" "$PKGS_REM"
	fi
	if $f_add && test -n "$PKGS_ADD";
	then
		verbose Add:
		printf "${BOLD}${FG_GREEN}%s${RESET}\n" "$PKGS_ADD"
	fi
	if $f_common && test -n "$PKGS_COM";
	then
		verbose Common:
		printf "${BOLD}${FG_BLUE}%s${RESET}\n" "$PKGS_COM"
	fi	
}

print_diff

if $f_diff;
then
	exit
fi

if $f_remove && test -n "$PKGS_REM";
then
	remove_packages
fi

if $f_add && test -n "$PKGS_ADD";
then
	add_packages
fi
