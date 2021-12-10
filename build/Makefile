NAME := test_shadowmocap

STAGE_DIR := ../stage
OUTPUT_DIR := $(STAGE_DIR)/obj
TARGET_DIR := $(STAGE_DIR)/bin
SOURCE_DIR := ../test

TARGET := $(TARGET_DIR)/$(NAME)

SRC := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJ := $(patsubst $(SOURCE_DIR)/%.cpp,$(OUTPUT_DIR)/%.o,$(SRC))

CPPFLAGS := \
-std=c++20 \
-I../include \
-I/Users/luke/xcompile/osx/local/include

test: builddir $(TARGET)
	$(TARGET)

$(TARGET): $(OBJ)
	clang++ $(LDFLAGS) -o $@ $<

$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	clang++ -c $(CPPFLAGS) $< -o $@

builddir:
	@install -d $(OUTPUT_DIR)
	@install -d $(TARGET_DIR)

.phony: builddir
