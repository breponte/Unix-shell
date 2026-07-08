#!/bin/bash

# define fail behavior
set -euo pipefail

original_cmd="pwd"
cmd="./bin/pwd"
path_test="./tests/pwd"
num_tests=$(wc -l < "$path_test/cases")
failed_tests=()

get_diff () {
    
    echo -e "----------------------\n\tTest $1\t\n----------------------"

    test_num=$1

    if diff $path_test/exp/exp$test_num $path_test/out/out$test_num >/dev/null; then
        echo -e "Test Passed!\n"
    else
        echo -e "Test Failed!\n"
        failed_tests+=($test_num)
    fi
}

echo -e "\nTest Suite for coreutils 'pwd'"

# since testing symlinks, manual editing of line 4 command necessary due to cd
sed "s/$original_cmd/.\/bin\/$original_cmd/" $path_test/cases > $path_test/my_cases

for i in $(seq 1 $num_tests); do
    sed "${i}q;d" "$path_test/cases" | bash > "$path_test/exp/exp$i"
    sed "${i}q;d" "$path_test/my_cases" | bash > "$path_test/out/out$i"
    get_diff $i
done

echo -e "----------------------\n\tSummary\t\n----------------------"
echo "Passed Tests: " "$(($num_tests-${#failed_tests[@]}))/$num_tests"
echo "Failed Tests: " "${failed_tests[@]}"
