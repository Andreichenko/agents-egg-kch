# Compiler & Flags
CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -O2 -Iinclude
NCURSES_FLAGS ?= -lncurses

# Target directories
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = tests

# Target binary
TARGET = $(BIN_DIR)/agent
TEST_TARGET = $(BIN_DIR)/test_suite

# Sources
SRCS = $(SRC_DIR)/main.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# OS Detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D_GNU_SOURCE
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D_DARWIN_C_SOURCE
endif

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) $(NCURSES_FLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_DIR)/test_dummy.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
