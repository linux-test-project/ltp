#!/bin/sh

testdir=$(realpath $(dirname $0))
export PATH="$PATH:$testdir:$testdir/tests/"

for i in `seq -w 01 06`; do
	echo
	echo "*** Running shell_test$i ***"
	echo
	./tests/shell_test$i
done

for i in shell_loader.sh shell_loader_all_filesystems.sh shell_loader_no_metadata.sh \
	 shell_loader_wrong_metadata.sh shell_loader_invalid_metadata.sh\
	 shell_loader_supported_archs.sh shell_loader_filesystems.sh\
	 shell_loader_tcnt.sh shell_loader_kconfigs.sh shell_loader_tags.sh \
	 shell_loader_invalid_block.sh shell_loader_c_child.sh; do
	echo
	echo "*** Running $i ***"
	echo
	$i
done

echo
echo "*** Testing LTP test -h option ***"
echo
shell_loader.sh -h

echo
echo "*** Testing LTP test -i option ***"
echo
shell_loader.sh -i 2
