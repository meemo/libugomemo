#!/bin/bash

#
# This file is a temporary compilation procedure. When the library is complete
# (and the file structure is finalized), a proper build system will be set up.
#

# Prepare build folders
mkdir -p ../build/
rm ../build/*
mkdir -p ../build/lib/

# Compile tomsfastmath
cd ../lib/tomsfastmath-0.13.1/
make -j 8
mv libtfm.a ../../build/lib/libtfm.a

# Compile zlib
cd ../zlib-1.2.11/
./configure
make -j 8
mv libz.a ../../build/lib/libz.a

# Compile libtomcrypt
cd ../libtomcrypt-1.18.2/
make -j 8 LTC_CFLAGS+="-I../tomsfastmath-0.13.1/src/headers/ -L../../build/lib/ -ltfm"
mv libtomcrypt.a ../../build/lib/libtomcrypt.a

# Compile src
cd ../../src/
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c io.cpp           -o ../build/io.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c kwz_audio.cpp    -o ../build/kwz_audio.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c kwz_video.cpp    -o ../build/kwz_video.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c kwz_sections.cpp -o ../build/kwz_sections.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ppm_audio.cpp    -o ../build/ppm_audio.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ppm_video.cpp    -o ../build/ppm_video.o
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -c ppm_meta.cpp     -o ../build/ppm_meta.o

# Build tests
cd ../tests/
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 kwz_test.cpp -o kwz_test
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 ppm_test.cpp -o ppm_test
g++ -std=c++11 -march=native -Wall -Wextra -Wpedantic -g -O3 -o ../tests/test test.cpp ../build/io.o \
-I../lib/libtomcrypt-1.18.2/src/headers/ -I../lib/tomsfastmath-0.13.1/src/headers/ -L../build/lib/ -ltomcrypt -ltfm
