# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude # -Iinclude tells the compiler where to find .h files

# Define the executable name
TARGET = main

# Define source and object files
SRCS = src/main.c src/lexer.c
OBJS = $(SRCS:.c=.o) # Replaces .c with .o to get the object file names (src/main.o src/lexer.o)

# --- Default Target: Build the Executable ---
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# --- Rule to compile .c files into .o files ---
# VPATH tells make to look in 'src' for prerequisites (like the .c files)
VPATH = src

# The pattern rule: how to make a .o from a .c
%.o: %.c include/lexer.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Target for Running the Program ---
# 'run' target with an optional argument for file input
# Usage: make run ARG=my_file.txt
run: $(TARGET)
	@echo "--- Running $(TARGET) ---"
ifdef ARG
	./$(TARGET) $(ARG)
else
	./$(TARGET)
endif
	@echo "-------------------------"

# --- Utility Targets ---
# Clean up the generated files
clean:
	rm -f $(TARGET) $(OBJS)
	@echo "Cleaned up $(TARGET) and object files."

.PHONY: all clean run