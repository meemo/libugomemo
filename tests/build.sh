#!/bin/bash

#
# This file is a temporary compilation procedure. When the library is complete, a proper build
# system will be set up.
#

# Prepare build folder
mkdir -p ../build/
rm ../build/*

# Compile all src/ files
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/io.cpp -o ../build/io.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/kwz_audio.cpp -o ../build/kwz_audio.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/kwz_video.cpp -o ../build/kwz_video.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/kwz_sections.cpp -o ../build/kwz_sections.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/ppm_audio.cpp -o ../build/ppm_audio.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/ppm_video.cpp -o ../build/ppm_video.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 -c ../src/ppm_meta.cpp -o ../build/ppm_meta.o

# Build tests
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 kwz_test.cpp -o kwz_test
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -O3 ppm_test.cpp -o ppm_test
