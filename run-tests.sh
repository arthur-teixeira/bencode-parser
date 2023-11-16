#!/bin/sh

CFLAGS="-Wall -Wextra -ggdb"

set -xe;

mkdir -p ./bin/

clang $CFLAGS -o ./bin/tests ./*.c -lunity

./bin/tests
