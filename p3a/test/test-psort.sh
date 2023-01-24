#! /bin/bash

if ! [[ -x psort ]]; then
    echo "psort executable does not exist"
    exit 1
fi
if ! [[ -d ./tests-out ]]; then
    mkdir ./tests-out
fi
export TESTS_FOLDER=/u/c/s/cs537-1/tests/p3a/tests
export TESTER_FOLDER=/u/c/s/cs537-1/tests/tester
export OUTPUT_FOLDER=./tests-out

$TESTER_FOLDER/run-tests.sh -d $TESTS_FOLDER $*
