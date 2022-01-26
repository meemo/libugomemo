#!/bin/bash

# Temporary compilation script. A proper makefile will be made later.

mkdir -p build/
cd build/ || exit 1

gcc -std=c89 -Wall -Wextra -Werror -pedantic -O3 -c -I../include/ ../src/*.c
gcc -std=c89 -Wall -Wextra -Werror -pedantic -O3 -c -I../include/ ../src/*/*.c

ar rcs ../libugomemo.a ./*.o
