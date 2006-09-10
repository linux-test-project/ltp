#!/bin/sh

output="linux_syscall_numbers.h"

rm -f ${output}

cat << EOF > ${output}
/*
 * Here we stick all the ugly *fallback* logic for linux
 * system call numbers (those __NR_ thingies).
 *
 * Licensed under the GPLv2 or later, see the COPYING file.
 */

#ifndef __LINUX_SYSCALL_NUMBERS_H__
#define __LINUX_SYSCALL_NUMBERS_H__

#include <sys/syscall.h>

EOF

for arch in `cat order` ; do
	echo -n "Generating data for arch $arch ... "

	echo "" >> ${output}
	echo "#ifdef __${arch}__" >> ${output}
	while read line ; do
		set -- $line
		nr=$1
		shift
		cat <<-EOF >> ${output}
		# ifndef $nr
		#  define $nr $*
		# endif
		EOF
	done < ${arch}.in
	echo "#endif" >> ${output}
	echo "" >> ${output}

	echo "OK!"
done

echo -n "Generating stub list ... "
echo "" >> ${output}
for nr in `cat stub-list` ; do
	cat <<-EOF >> ${output}
	# ifndef $nr
	#  define $nr 0
	# endif
	EOF
done
echo "" >> ${output}
echo "OK!"

echo "" >> ${output}
echo "#endif" >> ${output}
