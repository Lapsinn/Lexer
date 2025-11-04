# --- Makefile for C Lexer Project using .ec source files ---

SHELL = /bin/bash 

# Compiler and Flags
CC = gcc
# -Iinclude tells the compiler where to find your header file (lexer.h)
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = 

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

# Target executable name
TARGET = $(BIN_DIR)/lex_analyzer

# Source and Object Files
# Finds all .ec files in the src directory, treating them as source code
SRCS = $(wildcard $(SRC_DIR)/*.ec)
# Substitutes the .ec extension with .o and changes the directory from src to obj
OBJS = $(patsubst $(SRC_DIR)/%.ec,$(OBJ_DIR)/%.o,$(SRCS))

# --- Rules ---

# Default target: make all
.PHONY: all
all: directories $(TARGET)

# Rule 1: Link the object files to create the executable
$(TARGET): $(OBJS)
	@echo "🔗 Linking executable: $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Rule 2: Compile .ec source files into object files
# This pattern rule now explicitly handles the compilation of .ec files.
# We instruct GCC to compile it as a C file using the -x c flag.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.ec $(INC_DIR)/lexer.h
	@echo "🔨 Compiling C source file $< (as .ec) -> $@"
	# -x c tells GCC to treat the input file as C code, regardless of extension
	$(CC) $(CFLAGS) -x c -c $< -o $@

# Rule 3: Create necessary directories
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

# Rule 4: Clean up build files
.PHONY: clean
clean:
	@echo "🧹 Cleaning up build directories..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)