#!/bin/bash

#------------------------------------------------#
# Temporary compilation script(tm).              #
# Pass an argument to skip building the library. #
#------------------------------------------------#

build_dir=$(pwd)/build

# Compiler options
cc="clang"
common_flags="-std=c89 -pedantic -Wall -Wextra -g"
optimization="-march=native -O3"
include="-I$(pwd)/include"

include_library="-L$(pwd) -lugomemo"

cc_command="$cc $common_flags $optimization $include"

TIMEFORMAT="Finished in %R seconds."
time {
    mkdir -p $build_dir
    
    echo "Compiling library..."
    (
        cd src || exit 1
        
        for file in *.c; do
            $cc_command -c $file -o $build_dir/"${file%%.*}".o
        done
    )

    ar -r libugomemo.a $build_dir/*.o

    echo "Compiling tests..."
    (
        cd tests || exit 1

        for file in *.c; do
            $cc_command $file $include_library -o "${file%%.*}"
        done
    )

    echo "Cleaning up..."
    rm -f $build_dir/*.o
    rmdir $build_dir
}
