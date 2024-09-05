#!/bin/sh

set -eu

gcc        -std=c99 -Wall -Werror -Wextra -pedantic -O3 ./main.c -o main
gcc -DTEST -std=c99 -Wall -Werror -Wextra -pedantic -O3 ./main.c -o test

echo "== Standard Execution Output =="
./main
echo "\n== Test Execution Output =="
./test
