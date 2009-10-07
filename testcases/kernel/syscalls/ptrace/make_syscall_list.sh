#!/bin/sh
set -- $( \
	printf '#include <sys/syscall.h>' | \
	${CC:-gcc} -E -dD - | \
	awk '$2 ~ /^SYS_/ { sub(/SYS_/,"",$2); print $2; }'
)
printf 'P(%s)\n' "$@"
