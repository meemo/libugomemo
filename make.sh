#!/bin/bash

#------------------------------------------------#
# Temporary compilation script(tm).              #
# Pass an argument to skip building the library. #
#------------------------------------------------#

build_dir=$(pwd)/build

# Compiler options
cc="clang"
common_flags="-std=c89 -pedantic -Wall -Wextra -Werror -g"
optimization="-march=native -O3"
include="-I../include"

cc_command="$cc $common_flags $optimization $include"

TIMEFORMAT="Finished in %R seconds."
time {
    # Compile the library only if no arguments were passed.
    if (($# == 0)); then
        mkdir -p $build_dir

        (
            cd $build_dir || exit 1

            echo "Compiling library..."
            $cc_command -c ../src/*.c
            $cc_command -c ../src/*/*.c
        )

        ar -r libugomemo.a $build_dir/*.o

        echo "Cleaning up..."
        rm $build_dir/*.o
        rmdir $build_dir
    fi

    # Only build tests if the library has been compiled
    if [ -e libugomemo.a ]; then
        echo "Compiling tests..."
        $cc_command tests/sha1_test.c      -Iinclude -L. -lugomemo -o tests/sha1_test
        $cc_command tests/ppm_audio_test.c -Iinclude -L. -lugomemo -o tests/ppm_audio_test
        $cc_command tests/misc_test.c      -Iinclude -L. -lugomemo -o tests/misc_test
    else
        echo "libugomemo.a not found! Have you run the script without any parameters yet?"
    fi
}
