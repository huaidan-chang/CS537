#! /bin/bash

export TESTS_FOLDER=/u/c/s/cs537-1/tests/p1b/tests
export TESTER_FOLDER=/u/c/s/cs537-1/tests/tester

$TESTER_FOLDER/run-tests.sh -d $TESTS_FOLDER $* 

