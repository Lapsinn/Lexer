#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

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
    char *buffer = (char *)malloc(file_size + 1);
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
    printf("File size: %ld\n", file_size);
    printf("Last 50 chars: [%s]\n", buffer + (file_size > 50 ? file_size - 50 : 0));
    fclose(file);

    // Call the lex function (to be implemented)
    int result = lex(&lexer);

    if (result == 0) {
        printf("Lexing successful\n");
        printLexerTokens(&lexer);
    } else {
        printf("Lexing failed\n");
    }

    // Free the buffer
    free(buffer);
    free_lexer(&lexer);
    return 0;
}