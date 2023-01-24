#! /bin/bash

if ! [[ -x wish ]]; then
    echo "wish executable does not exist"
    exit 1
fi
export TESTS_FOLDER=/u/c/s/cs537-1/tests/p2a/tests
export TESTER_FOLDER=/u/c/s/cs537-1/tests/tester

$TESTER_FOLDER/run-tests.sh -d $TESTS_FOLDER $*



