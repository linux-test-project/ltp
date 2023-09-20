#!/bin/sh
# Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>

ask()
{
	local msg="$1"
	local answer

	printf "\n$msg. Proceed? [N/y]: "
	read answer
	case "$answer" in
		[Yy]*) : ;;
		*) exit 2
	esac
}

quit()
{
	printf "\n$@\n" >&2
	exit 1
}

rod()
{
	eval "$@" || quit "$@ failed"
}

title()
{
	echo "===== $1 ====="
}
