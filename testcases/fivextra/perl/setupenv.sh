#!/bin/bash
# Robb Romans <robb@austin.ibm.com>
PERL_VER=$(perl -v | grep 'v5' | cut -d' ' -f4 | cut -c2-6)

if [[ ! -L t/perl ]] ; then
	ln -s $(which perl) t/perl
fi

if [[ ! -L lib ]] ; then
	ln -s /usr/lib/perl5/$PERL_VER lib
fi

for dir in t/* ; do
	if [[ -d $dir ]] && [[ ! -L $dir/perl ]] ; then
		ln -s $(which perl) $dir/perl
	fi
done

exit 0

