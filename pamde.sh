#!/bin/bash

set -e

# TODO:
## improve confirmation (yay style)

profile_section_begin=$(date +%s%N)

profile_begin()
{
	if ! $f_profile; then
		return
	fi
	profile_section_end=$(date +%s%N)
}

profile_end()
{
	if ! $f_profile; then
		return
	fi
	section=$1
	profile_section_end=$(date +%s%N)
	echo "-p: $section took $(( (profile_section_end - profile_section_begin)/1000000 )) ms"
	profile_section_begin=$(date +%s%N)
}

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

profile_end init

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

profile_begin

SYSTEM_LIST=$(pacman -Qqe)

profile_end query

parse_file()
{
	local file="$1"
	local base_dir=$(dirname "$file")
	while IFS= read -r line || test -n "$line";
	do
		local include=false
		for word in $line;
		do
			if $include;
			then
				parse_file "$base_dir/$word"
				include=false
			fi
			case $word in
			\#*)
				continue 2
			;;
			!include)
				include=true
				continue 1
			;;
			esac
			SOURCE_LIST="$SOURCE_LIST $word"
		done

	done < "$file"
}

parse_file "$SOURCE"

profile_end config

SOURCE_LIST=$(echo $SOURCE_LIST | tr -s '[:space:]' '\n' | sort)
SYSTEM_LIST=$(echo $SYSTEM_LIST | tr -s '[:space:]' '\n' | sort)

PKGS_ADD=$(comm -23 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))
PKGS_REM=$(comm -13 <(echo "$SOURCE_LIST") <(echo "$SYSTEM_LIST"))

profile_end preprocess

if $f_clean;
then
	PKGS_REM="$PKGS_REM $(pacman -Qdtq || true)"
	profile_end add_clean
fi

PKGS_ADD=$(echo $PKGS_ADD)
PKGS_REM=$(echo $PKGS_REM)

installed=$(pacman -Qq)

PKGS_ADD_OLD=$PKGS_ADD
PKGS_ADD=""
PKGS_IGNORED=""
for pkg in $PKGS_ADD_OLD;
do
	if echo "$installed" | grep -qx "$pkg";
	then
		PKGS_IGNORED="$PKGS_IGNORED $pkg"
	else
		PKGS_ADD="$PKGS_ADD $pkg"
	fi
done

PKGS_ADD=$(echo $PKGS_ADD)

profile_end filter

confirm()
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
