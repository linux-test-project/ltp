#!/bin/bash
#
#	place names of supported tinylogin functions into script
#
if ! type cat >&/dev/null && type sed >&/dev/null ; then
	echo *****************************************
	echo "Exiting: need cat and sed to continue!"
	echo *****************************************
#	exit 1
fi

names=""
for n in `cat tinylogin.links` ; do
	names="$names ${n##*/}" # append basename of link
done

sed "/PUTNAMESHERE/s/PUTNAMESHERE/$names/" uncustomized_tinylogin.sh \
	 > tinylogin.sh

chmod +x tinylogin.sh

