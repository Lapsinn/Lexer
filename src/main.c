#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parse.h"
#include "ast.h"

// ============================================================================
// Configuration & Enums
// ============================================================================

typedef enum {
    MODE_LEXER,
    MODE_PARSER
} CompilerMode;

// ============================================================================
// File Utilities
// ============================================================================

static int has_ec_extension(const char* filename) {
    if (!filename) return 0;
    size_t len = strlen(filename);
    return len >= 3 && strcmp(filename + len - 3, ".ec") == 0;
}

static char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        perror("Error checking file size");
        fclose(f);
        return NULL;
    }

    long size = ftell(f);
    if (size < 0) {
        perror("Error checking file size");
        fclose(f);
        return NULL;
    }
    rewind(f);

    char* buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fprintf(stderr, "Error: Memory allocation failed for file buffer\n");
        fclose(f);
        return NULL;
    }

    size_t n = fread(buf, 1, (size_t)size, f);
    if (n < (size_t)size) {
        fprintf(stderr, "Error: Failed to read entire file\n");
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);

    buf[n] = '\0';
    return buf;
}

void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage: %s <input.ec> <output.txt> [options]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -l, --lexer    Run lexer only (prints tokens)\n");
    fprintf(stderr, "  -p, --parser   Run parser (prints AST) [Default]\n");
}

// ============================================================================
// Main Execution
// ============================================================================

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_path = argv[2];
    CompilerMode mode = MODE_PARSER; // Default behavior

    // Parse optional arguments
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lexer") == 0) {
            mode = MODE_LEXER;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parser") == 0) {
            mode = MODE_PARSER;
        } else {
            fprintf(stderr, "Warning: Unknown option '%s'\n", argv[i]);
        }
    }

    // Validate Input Extension
    if (!has_ec_extension(input_path)) {
        fprintf(stderr, "Error: Input file must have .ec extension\n");
        return 1;
    }

    // Read Source
    char* source = read_file(input_path);
    if (!source) {
        return 1;
    }

    // Initialize Lexer
    Lexer lexer;
    // Zero-init the struct to be safe
    memset(&lexer, 0, sizeof(Lexer));
    
    lexer.start_tok  = source;
    lexer.cur_tok    = source;
    lexer.line_start = source;
    lexer.line_number = 1;
    lexer.token_count = 0;
    lexer.capacity = 0;
    lexer.tokens = NULL; 

    // Run Lexer (Phase 1)
    // We run this regardless of mode, as Parser needs tokens.
    if (lex(&lexer) != 0) {
        fprintf(stderr, "Fatal Error: Lexing failed.\n");
        free(source);
        free_lexer(&lexer);
        return 1;
    }

    // Redirect stdout to the output file
    // This allows existing print functions (printLexerTokens, print_ast) to work unchanged.
    if (freopen(output_path, "w", stdout) == NULL) {
        fprintf(stderr, "Error: Could not open output file '%s' for writing.\n", output_path);
        free(source);
        free_lexer(&lexer);
        return 1;
    }

    // Execute Mode Logic
    if (mode == MODE_LEXER) {
        printf("=== Lexer Output ===\n");
        printf("Source File: %s\n", input_path);
        printf("Token Count: %zu\n\n", lexer.token_count);
        printLexerTokens(&lexer);
    } 
    else {
        // MODE_PARSER
        printf("=== Parser Output (AST) ===\n");
        printf("Source File: %s\n\n", input_path);

        Parser* parser = parser_create(lexer.tokens, lexer.token_count);
        if (!parser) {
            // Should theoretically not happen unless malloc fails
            fprintf(stderr, "Fatal Error: Failed to create parser.\n");
            fclose(stdout); // Close file handle
            free(source);
            free_lexer(&lexer);
            return 1;
        }

        ASTNode* program = parse_program(parser);

        if (program && !parser->has_error) {
            printf("Parsing Status: SUCCESS\n\n");
            print_ast(program, 0);
        } else {
            printf("Parsing Status: FAILED\n");
            printf("Check console (stderr) for syntax error details.\n");
        }

        if (program) free_ast(program);
        parser_destroy(parser);
    }

    // Cleanup
    fclose(stdout); // Close output file
    free_lexer(&lexer);
    free(source);

    return 0;
}