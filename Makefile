# Compiler & Flags
CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -O2 -Iinclude
NCURSES_FLAGS ?= -lncurses

# Target directories
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = tests

# Target binaries
TARGET = $(BIN_DIR)/agent
TEST_TARGET = $(BIN_DIR)/test_suite

# OS Detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D_GNU_SOURCE
    SYS_SRC = $(SRC_DIR)/sys/sys_linux.c
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D_DARWIN_C_SOURCE
    SYS_SRC = $(SRC_DIR)/sys/sys_darwin.c
endif

# Sources
SRCS = $(SRC_DIR)/main.c $(SYS_SRC)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Test sources
TEST_OBJS = $(BUILD_DIR)/sys/$(notdir $(SYS_SRC:.c=.o))

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) $(NCURSES_FLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_DIR)/test_metrics.c $(TEST_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_metrics.c $(TEST_OBJS) -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
