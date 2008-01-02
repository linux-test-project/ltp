#!/bin/sh

# Update the reference policy as needed to suite local policy

perl ./check_sbin_deprecated.pl
ret=$?
if [ $ret -eq 0 ]; then
  echo "Leaving reference policy unchanged"
elif [ $ret -eq 2 -a ! -f PATCHED ]; then
  touch PATCHED
  pushd ../refpolicy
  echo "Updating reference policy for Fedora 8"
  patch -p1 < ../misc/sbin_deprecated.patch
  popd
else
  echo "Error looking through local policy."
  echo "Leaving test policy unchanged"
fi
