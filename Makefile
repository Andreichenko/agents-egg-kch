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
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/ui/ui.c $(SYS_SRC)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Test sources
SYS_OBJ = $(BUILD_DIR)/sys/$(notdir $(SYS_SRC:.c=.o))
UI_OBJ = $(BUILD_DIR)/ui/ui.o

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) $(NCURSES_FLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

test: $(BIN_DIR)/test_metrics $(BIN_DIR)/test_ui
	@./$(BIN_DIR)/test_metrics
	@./$(BIN_DIR)/test_ui

$(BIN_DIR)/test_metrics: $(TEST_DIR)/test_metrics.c $(SYS_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_metrics.c $(SYS_OBJ) -o $@

$(BIN_DIR)/test_ui: $(TEST_DIR)/test_ui.c $(UI_OBJ) $(SYS_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_ui.c $(UI_OBJ) $(SYS_OBJ) $(NCURSES_FLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
