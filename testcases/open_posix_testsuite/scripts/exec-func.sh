#!/usr/bin/env bash

for d in $(./locate-test --frun); do
    echo "Testing $d"
    pushd $d >/dev/null
    ./run.sh
    popd >/dev/null
done
