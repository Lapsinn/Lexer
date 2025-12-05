#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Check the file extension 
    const char *filename = argv[1];
    int len = strlen(filename);
    
    // Check if the length is at least 3 
    // AND if the last 3 characters are ".ec"
    if (len < 3 || strcmp(filename + len - 3, ".ec") != 0) {
        fprintf(stderr, "Error: File must have the .ec extension (e.g., sample.ec).\n");
        return 1;
    }

    // Open the file
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate buffer
    char *buffer = (char *)calloc(file_size + 1, sizeof(char));
    if (!buffer) {
        perror("Error allocating memory");
        fclose(file);
        return 1;
    }
    
    Lexer lexer = {
        .start_tok = buffer, 
        .cur_tok = buffer, 
        .line_start = buffer,
    };

    // read the file into the buffer
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0'; // Null-terminate the string
    fclose(file);

    // Call the lex function (to be implemented)
    int result = lex(&lexer);

    if (result != 0) {
        printf("Lexing failed\n");
        free(buffer);
        free_lexer(&lexer);
        return 1;
    }

    printf("Lexing successful!\n");
    printf("Total tokens: %zu\n\n", lexer.token_count);
    printLexerTokens(&lexer);

    printf("\n=================================\n");
    printf("  SYNTAX ANALYSIS (PARSING)\n");
    printf("=================================\n");

    // Initialize parser with lexer tokens
    Parser parser;
    parser.tokens = lexer.tokens;
    parser.current = 0;
    parser.count = lexer.token_count;
    parser.has_error = 0;

    // Parse the program
    ASTNode* program = parse_program(&parser);

    if (parser.has_error || !program) {
        printf("\n[PARSING FAILED]\n");
        printf("Syntax errors detected. Cannot generate AST.\n");
        
        if (program) {
            free_ast(program);
        }
        free(buffer);
        free_lexer(&lexer);
        return 1;
    }

    printf("[PARSING SUCCESS]\n");
    printf("Successfully parsed the program!\n\n");

    printf("=================================\n");
    printf("  ABSTRACT SYNTAX TREE (AST)\n");
    printf("=================================\n\n");

    // Print the AST
    print_ast(program, 0);

    printf("\n=================================\n");
    printf("  CLEANUP\n");
    printf("=================================\n");

    // Free resources
    free_ast(program);
    printf("AST freed successfully\n");
    
    free_lexer(&lexer);
    printf("Lexer freed successfully\n");
    
    free(buffer);
    printf("Buffer freed successfully\n");

    printf("\n=================================\n");
    printf("  COMPILATION COMPLETE\n");
    printf("=================================\n");

    return 0;
}