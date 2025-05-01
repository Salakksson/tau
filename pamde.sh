#!/bin/bash

set -e

profile_init=$(date +%s%N)

print_help()
{
	echo "Usage: $0 [options] (file)"
	echo "Options:"
	echo "  -s       Sync (add/remove)"
	echo "  -c       Cleans up unused packages"
	echo "  -u       Use AUR"
	echo "  -q       Quiet mode"
	echo "  -d       Dry run"
	echo "  -f       No confirm (Scary!)"
	echo "  -p       Profile"
	echo "  -h       Display this help message"
	echo " --add     Add packages"
	echo " --remove  Remove packages"
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


f_add=false
f_remove=false
f_clean=false
f_aur=false
f_verbose=true
f_dry=false
f_force=false
f_profile=false

SOURCE="/usr/local/share/pamde/main.conf"
AUR_CMD="yay"

file_set=false
while [ "$#" -gt 0 ]
do
	arg="$1"
	case $arg in
	--add) f_add=true;;
	--remove) f_remove=true;;
	--clean) f_clean=true;;
	-*)
		flags=$(echo "$arg" | sed 's/-//')
		for c in $(echo "$flags" | fold -w1)
		do
			case "$c" in
			s)
				f_add=true
				f_remove=true
				;;
			c) f_clean=true ;;
			u) f_aur=true ;;
			q) f_verbose=false ;;
			d) f_dry=true ;;
			f) f_force=true ;;
			p) f_profile=true ;;
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

if $f_profile;
then
	profile_init_end=$(date +%s%N)
	echo "-p: init took $(( (profile_init_end - profile_init)/1000000 )) ms"
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

# main package lists
SOURCE_LIST=""
SYSTEM_LIST=""

if $f_profile;
then
	query=$(date +%s%N)
fi

# query system packages
SYSTEM_LIST=$(pacman -Qqe)

if $f_profile;
then
	query_end=$(date +%s%N)
	echo "-p: query took $(( (query_end - query)/1000000 )) ms"
	profile_config=$(date +%s%N)
fi

parse_file()
{
	file="$1"
	base_dir=$(dirname "$file")
	while IFS= read -r line || test -n "$line";
	do
		line=$(echo "$line" | sed -e 's/#.*//' -e 's/^\s*//')

		if ! test -n "$line";
		then
			continue
		fi

		case "$line" in
		!include*)
			include_file=$(echo "${line#*!include}" | sed -e 's/^ *//' -e 's/ *$//')
			parse_file "$base_dir/$include_file"
			;;
		*)
			SOURCE_LIST="$SOURCE_LIST $line"
			;;
		esac

	done < "$file"
}

parse_file "$SOURCE"

if $f_profile;
then
	profile_config_end=$(date +%s%N)
	echo "-p: config took $(( (profile_config_end - profile_config)/1000000 )) ms"
	profile_filter=$(date +%s%N)
fi

SOURCE_LIST=$(echo "$SOURCE_LIST" | sed 's/^ *//' | sed 's/\s\+/\n/g')
SYSTEM_LIST=$(echo "$SYSTEM_LIST" | sed 's/\s\+/\n/g')

SOURCE_LIST=$(echo "$SOURCE_LIST" | sort)
SYSTEM_LIST=$(echo "$SYSTEM_LIST" | sort) # dont rely on pacman to sort the list

PKGS_ADD=$(comm -23 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))
PKGS_REM=$(comm -13 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))

PKGS_ADD=$(echo $PKGS_ADD)
PKGS_REM=$(echo $PKGS_REM)

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

PKGS_ADD=$(echo $PKGS_ADD)

if $f_profile;
then
	profile_filter_end=$(date +%s%N)
	echo "-p: filter took $(( (profile_filter_end - profile_filter)/1000000 )) ms"
fi

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

clean_packages()
{
	PKGS_CLEAN=$(pacman -Qdtq || true)
	if test ! -n "$PKGS_CLEAN";
	then
		echo -e "${BOLD}${FG_GREEN}No packages to clean${RESET}"
		return 0
	fi
	# TODO: add option to ignore packages like in yay, probably in rest of the program aswell
	confirm pacman -Rsn $PKGS_CLEAN
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
}

print_diff

if $f_dry;
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

if $f_clean;
then
	clean_packages
fi
