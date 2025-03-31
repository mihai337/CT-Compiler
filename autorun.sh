#! /bin/bash

gcc -Wall compiler.c
echo ""

failed_count=0
test_count=0

# run ./a.out on all .c files in the tests directory
for file in ./tests/*.c
do
    test_count=$((test_count + 1))
    stderr_output=$(./a.out $file 2>&1 > ./tests/$(basename $file .c).out )
    if [ $? -ne 0 ]; then
        failed_count=$((failed_count + 1))
        echo "Error running $file"
        echo "$stderr_output"
        echo "------------------------"
    fi
done

echo "Successfully ran $((test_count - failed_count))/$((test_count)) tests"
