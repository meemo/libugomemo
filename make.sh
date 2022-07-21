#!/bin/bash

#
# Temporary compilation script(tm).
#

# Make the build directory.
mkdir -p build
cd build

# Remove any previously compiled objects.
rm *

# Build the library.
gcc -std=c89 -Wall -Wextra -Werror -pedantic -O3 -c -I../include/ ../src/*.c
gcc -std=c89 -Wall -Wextra -Werror -pedantic -O3 -c -I../include/ ../src/*/*.c

ar rcs ../libugomemo.a ./*.o

cd ..

# Build the tests.
gcc -std=c89 -Wall -Wextra -Werror -pedantic -O3 -Iinclude tests/sha1_test.c libugomemo.a -o tests/sha1_test
gcc -std=c89 -Wall -Wextra -Werror -pedantic -O3 -Iinclude tests/ppm_audio_test.c libugomemo.a -o tests/ppm_audio_test
