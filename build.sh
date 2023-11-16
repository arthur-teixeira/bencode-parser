#!/bin/sh

CFLAGS="-Wall -Wextra -ggdb"

set -xe;

clang $CFLAGS -o program  ./*.c -lunity

