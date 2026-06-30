#!/bin/bash

# define fail behavior
set -euo pipefail

# coreutils s for this test
cmd="./bin/echo"
path_test="./tests/echo"

echo "Test Suite for coreutils 'echo'"

echo -e "----------------------\n\tTest 1\t\n----------------------\n"

test_num=1
test_args="Hello World"
$cmd "$test_args" > $path_test/out/out$test_num

if diff $path_test/exp/exp$test_num $path_test/out/out$test_num >/dev/null; then
    echo "Test Passed!"
else
    echo "Test Failed!"
fi
