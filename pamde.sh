#!/bin/sh

set -e

print_help()
{
	echo "Usage: $0 [options] (file)"
	echo "Options:"
	echo "  -r       Remove packages"
	echo "  -a       Add packages"
	echo "  -d       Only show diff"
	echo "  -c       Show common packages"
	echo "  -f       No confirm (Scary!)"
	echo "  -q       Quiet mode"
	echo "  -u       Use AUR"
	echo "  -h       Display this help message"
	echo "(file) defaults to /usr/local/share/pamde/main.conf"
}

if [ $# -eq 0 ]; then
	print_help
	exit 0
fi

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

f_remove=false
f_add=false
f_diff=false
f_common=false
f_force=false
f_verbose=true
f_aur=false
SOURCE="/usr/local/share/pamde/main.conf"
AUR_CMD="yay"

file_set=false
while [ "$#" -gt 0 ]
do
	arg="$1"
	case $arg in
	-*)
		flags=$(sed 's/-//' <<< "$arg")
		for c in $(fold -w1 <<< "$flags")
		do
			case "$c" in
			r) f_remove=true ;;
			a) f_add=true ;;
			d) f_diff=true ;;
			c) f_common=true ;;
			f) f_force=true ;;
			q) f_verbose=false ;;
			u) f_aur=true ;;
			h)
				print_help
				exit 0
				;;
			*)
				echo "Unknown flag -$c"
				print_help
				exit 1
				;;
			esac
		done
		;;
	*)
		if $file_set;
		then
			echo "Unexpected argument: $arg"
			print_help
			exit 1
		fi
		SOURCE="$arg"
		file_set=true
		;;
	esac
	shift
done

verbose()
{
	if $f_verbose;
	then
		echo -e $@
	fi
}

if ! $file_set;
then
	verbose "Using default source '$SOURCE'"
fi

if ! test -e $SOURCE;
then
	echo -e "${BOLD}${FG_YELLOW}File '$SOURCE' does not exist"
	echo -e "Create it and fill with current packages? (y/N) $RESET"
	read responce
	case $responce in
		[y/Y]*)
			sudo mkdir -p $(dirname "$SOURCE")
			sudo chmod 777 $(dirname "$SOURCE")
			touch "$SOURCE"
			pacman -Qqe > "$SOURCE" ;;
		*) return 1 ;;
	esac
fi

if $f_aur;
then
	verbose using $AUR_CMD for AUR
fi

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
	old_dir=$(pwd)
	cd $(dirname $file)
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
	cd $old_dir # not using cd - since !incldues can be nested
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

is_package_installed() # package
{
	package=$1
	if pacman -Qi $package &> /dev/null;
	then
		return 0
	fi
	return 1
}

PKGS_ADD_OLD=$PKGS_ADD
PKGS_ADD=""
PKGS_IGNORED=""
for pkg in $PKGS_ADD_OLD;
do
	if is_package_installed $pkg;
	then
		PKGS_IGNORED="$PKGS_IGNORED $pkg"
	else
		PKGS_ADD="$PKGS_ADD $pkg"
	fi
done

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
	if $f_aur;
	then
		confirm yay -S $PKGS_ADD
	else
		confirm sudo pacman -S $PKGS_ADD
	fi
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

#verbose ignoring packages:
#verbose "$FG_YELLOW $PKGS_IGNORED $RESET"

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
