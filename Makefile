CC      := clang
CCFLAGS := -Wall -Wpedantic -O0 -g -fsanitize=address,undefined -Wno-gnu-zero-variadic-macro-arguments
LDFLAGS := -fsanitize=address,undefined -fno-sanitize=leak

BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj
DEPFILE := $(BUILD_DIR)/.deps
SRC_DIR := src
TARGET := $(BIN_DIR)/tauc

INSTALL := false
RUN := true

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(shell mkdir -p $(BUILD_DIR) $(BIN_DIR) $(OBJ_DIR))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -MMD -MP -c $< -o $@

-include $(OBJ_DIR)/*.d

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* $(BUILD_DIR)/.deps

.PHONY: run
run: $(TARGET)
	if $(RUN); then ./$(TARGET); fi

.PHONY: rebuild
rebuild: clean all
