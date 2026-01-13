

# EC Language Compiler Frontend

A C-based compiler frontend for the `.ec` programming language. This project includes a Lexer for tokenization and a Recursive Descent Parser that constructs an Abstract Syntax Tree (AST).

## Features

* **Lexical Analysis:** Converts raw source code into a stream of tokens.
* **Parsing:** Implements a recursive descent parser with **Panic Mode** error recovery.
* **AST Generation:** Builds a structured tree representation of the code (Programs, Statements, Expressions).

## Prerequisites

* **GCC (GNU Compiler Collection)**
    * Ensure `gcc` is installed and available in your system `PATH`.

## Project Structure

```text
project/
├── include/          # Header files (lexer.h, parser.h, ast.h, etc.)
├── src/
│   ├── main.c        # Entry point (drivers for lexer/parser)
│   ├── lexer.c       # Tokenization implementation
│   ├── parser.c      # Parser implementation
│   └── ast.c         # AST node definitions and helpers
├── sample.ec         # Sample input file
└── README.md         # This file

```

## Installation & Usage

### 1. Clone the Repository

```bash
git clone <repository-url>
cd <repository-name>

```

### 2. Compile the Program

Since the project includes multiple source files (Lexer, Parser, AST), use the wildcard `*.c` to compile everything in the `src` directory. We strictly recommend using warning flags to catch potential bugs early.

```bash
gcc -Wall -Wextra -g -Iinclude src/*.c -o my_program.exe

```

**Compilation flags explained:**

* **`-Wall` (Warn All):** Despite the name, this turns on the *most important* warnings. It catches unused variables, uninitialized variable usage, and simple logic errors.
* **`-Wextra` (Warn Extra):** Enables extra warnings that `-Wall` leaves out. It is useful for catching unused function parameters (common in parser callbacks), sign comparison issues, and empty statements.
* **`-g` (Debug):** Includes debug symbols in the executable. This allows debuggers (like GDB or LLDB) to point to the exact file and line number if the program crashes, rather than just a memory address.
* **`-Iinclude`:** Adds the `include/` directory to the header search path.
* **`src/*.c`:** Compiles all C source files found in `src/`.
* **`-o my_program.exe`:** Specifies the output executable name.

### 3. Run the Program

**On Linux/macOS:**

```bash
./my_program sample.ec

```

**On Windows:**

```cmd
my_program.exe sample.ec

```

## Error Handling

The parser implements **Panic Mode Recovery**. If a syntax error is encountered, the parser will:

1. Report the error line and message.
2. Synchronize its state by skipping tokens until it finds a statement boundary (like `;` or `}`).
3. Continue parsing the rest of the file to report any further errors.

```

```
