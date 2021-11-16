#!/bin/bash

#
# This file is a temporary compilation procedure. When the library is complete
# (and the file structure is finalized), a proper build system will be set up.
#

# Prepare build folder
mkdir -p ../build/
rm ../build/*

# Compile src
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/io.cpp           -o ../build/io.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/kwz_audio.cpp    -o ../build/kwz_audio.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/kwz_video.cpp    -o ../build/kwz_video.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/kwz_sections.cpp -o ../build/kwz_sections.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/ppm_audio.cpp    -o ../build/ppm_audio.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/ppm_video.cpp    -o ../build/ppm_video.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ../src/ppm_meta.cpp     -o ../build/ppm_meta.o

# Compile tests
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c kwz_test.cpp            -o ../build/kwz_test.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ppm_test.cpp            -o ../build/ppm_test.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c test.cpp                -o ../build/test.o

# Build tests
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 ../build/kwz_test.o -o kwz_test
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 ../build/ppm_test.o -o ppm_test
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 ../build/io.o ../build/kwz_sections.o ../build/test.o -o test
