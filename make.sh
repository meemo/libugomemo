#!/bin/bash

#------------------------------------------------#
# Temporary compilation script(tm).              #
# Pass an argument to skip building the library. #
#------------------------------------------------#

build_dir=$(pwd)/build

# Compiler options
cc="gcc"
common_flags="-std=c89 -pedantic -Wall -Wextra -Werror"
optimization="-march=native -O3"
include="-I../include"

cc_command="$cc $common_flags $optimization $include"

TIMEFORMAT="Done in %R seconds."
time {
    mkdir -p $build_dir

    (
        cd $build_dir || exit

        # Compile the library only if no arguments were passed.
        if (($# == 0)); then
            echo "Compiling library..."
            $cc_command -c ../src/*.c   || exit
            $cc_command -c ../src/*/*.c || exit

            ar rcs ../libugomemo.a ./*.o
        fi

        echo "Compiling tests..."
        $cc_command ../tests/sha1_test.c      ../libugomemo.a -o ../tests/sha1_test
        $cc_command ../tests/ppm_audio_test.c ../libugomemo.a -o ../tests/ppm_audio_test
    )

    echo "Cleaning up..."
    rm $build_dir/*.o
    rmdir $build_dir
}
