CC := $(shell command -v clang 2>/dev/null || echo gcc)
CFLAGS_COMMON = -std=c99 -Wall -Wextra -Wno-unused-const-variable -Iinclude
STATIC_TARGET = libugomemo.a
SRC_DIR = src
BUILD_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Platform detection for shared library extension
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHARED_TARGET = libugomemo.dylib
    SHARED_FLAGS = -dynamiclib
else
    SHARED_TARGET = libugomemo.so
    SHARED_FLAGS = -shared
endif

RELEASE_DIR = $(BUILD_DIR)/release
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(RELEASE_DIR)/%.o,$(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c,$(DEBUG_DIR)/%.o,$(SRCS))
# Shared library needs position-independent code
SHARED_DIR = $(BUILD_DIR)/shared
SHARED_OBJS = $(patsubst $(SRC_DIR)/%.c,$(SHARED_DIR)/%.o,$(SRCS))

MAKEFLAGS += -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

CFLAGS_RELEASE = -O3 -march=native -flto
CFLAGS_DEBUG = -O0 -ggdb3
CFLAGS_SHARED = -O3 -march=native -fPIC

HOMEBREW_PREFIX = /opt/homebrew
HOMEBREW_EXISTS := $(shell test -d $(HOMEBREW_PREFIX) && echo "yes" || echo "no")

ifeq ($(HOMEBREW_EXISTS),yes)
    LDFLAGS += -L$(HOMEBREW_PREFIX)/opt/openssl@3/lib
    LDFLAGS += -L$(HOMEBREW_PREFIX)/opt/zlib/lib
    LDFLAGS += -L$(HOMEBREW_PREFIX)/opt/gmp/lib

    CFLAGS_COMMON += -I$(HOMEBREW_PREFIX)/opt/openssl@3/include
    CFLAGS_COMMON += -I$(HOMEBREW_PREFIX)/opt/zlib/include
    CFLAGS_COMMON += -I$(HOMEBREW_PREFIX)/opt/gmp/include
endif

LIBS = -lz -lgmp -lcrypto

release: $(RELEASE_DIR)/$(STATIC_TARGET)

debug: $(DEBUG_DIR)/$(STATIC_TARGET)

shared: $(SHARED_DIR)/$(SHARED_TARGET)

all: release debug shared

# Static library objects
$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -c $< -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -c $< -o $@

# Shared library objects (with -fPIC)
$(SHARED_DIR)/%.o: $(SRC_DIR)/%.c | $(SHARED_DIR)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_SHARED) -c $< -o $@

# Static library
$(RELEASE_DIR)/$(STATIC_TARGET): $(RELEASE_OBJS)
	ar rcs $@ $^

$(DEBUG_DIR)/$(STATIC_TARGET): $(DEBUG_OBJS)
	ar rcs $@ $^

# Shared library
$(SHARED_DIR)/$(SHARED_TARGET): $(SHARED_OBJS)
	$(CC) $(SHARED_FLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

$(SHARED_DIR):
	mkdir -p $(SHARED_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean release debug shared
