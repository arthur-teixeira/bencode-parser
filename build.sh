#!/bin/sh

CFLAGS="-Wall -Wextra -ggdb"

set -xe;

clang $CFLAGS *.c -o program
