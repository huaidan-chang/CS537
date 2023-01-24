#!/usr/bin/env bash

set -e
TESTSCRIPT_DIR="/home/cs537-1/tests/p1a"
TESTSCRIPT="$TESTSCRIPT_DIR/test-p1a.py"

make || (echo "Unable to compile code. Are you running this from your source code folder? Is the make file there?" && exit 1)
[ -d t ] || (echo "Unable to find directory 't'. Make sure your current working directory is memcached-1.6.17" && exit 1)
mkdir -p t/ignored
[ -f t/maxconns.t ] && mv t/maxconns.t t/ignored/maxconns.t # this test uses network ports instead of unix sockets. Disable it to allow tests to work in CSL machines.
[ -f t/issue_67.t ] && mv t/issue_67.t t/ignored/issue_67.t # this test uses network ports instead of unix sockets. Disable it to allow tests to work in CSL machines.
[ -f t/startfile.lua ] && mv t/startfile.lua t/ignored/startfile.lua # this test uses network ports instead of unix sockets. Disable it to allow tests to work in CSL machines.

if [[ $# -eq 0 ]]
  then
    python3 $TESTSCRIPT -v
elif [ $# -eq 1 ] && [ $1 == "--full" ]
  then
    FULL_TEST_MODE=1 python3 $TESTSCRIPT -v
else
    echo "Usage: run-tests.sh [--full]"
fi
