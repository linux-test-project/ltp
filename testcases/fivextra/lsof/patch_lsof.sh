#!/bin/bash
testlist=`cat testlist`
cat > sedscript <<-EOF
	#!`which sed` -f
	s/@TESTLIST@/$testlist/
EOF
chmod +x sedscript
./sedscript unpatched_lsof.sh > lsof.sh
chmod +x lsof.sh
rm sedscript
