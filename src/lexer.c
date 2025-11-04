#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lex(Lexer *lexer) {
    while (*lexer->cur_tok != '\0') {
         switch (*lexer->cur_tok) {
            case ' ':
                // do nothing
                break;
            case '\n':
                lexer->line_number++;
                lexer->line_start = lexer->cur_tok + 1;
                break;
            case '(':
                add_token(lexer, TOKEN_LPAREN, "(");
                break;
            case ')':
                add_token(lexer, TOKEN_RPAREN, ")");
                break;
            case '{':
                add_token(lexer, TOKEN_LBRACE, "{");
                break;
            case '}':
                add_token(lexer, TOKEN_RBRACE, "}");
                break;
            case '[':
                add_token(lexer, TOKEN_LBRACKET, "[");
                break;
            case ']':
                add_token(lexer, TOKEN_RBRACKET, "]");
                break;
            case ',':
                add_token(lexer, TOKEN_COMMA, ",");
                break;
            case ';':
                add_token(lexer, TOKEN_SEMICOLON, ";");
                break;
            case '#':
                while (*lexer->cur_tok != '\n' && *lexer->cur_tok != '\0') lexer->cur_tok++;
                continue;
            default:
                if (isdigit(*lexer->cur_tok)) {
                    char *str_start = lexer->cur_tok;
                    int decimal_count = 0;
                    int digits_after_decimal = 0;
                    int in_decimal_part = 0;

                    while (isdigit(*lexer->cur_tok) || *lexer->cur_tok == '.') {
                        if (*lexer->cur_tok == '.') {
                            decimal_count++;
                            if (decimal_count > 1) {
                                fprintf(stderr, "%zu: Invalid number format — multiple decimal points\n", lexer->line_number);
                                return 1;
                            }
                            in_decimal_part = 1;
                        }
                        lexer->cur_tok++;
                    }

                    int str_len = lexer->cur_tok - str_start;
                    char *number = (char *)malloc(str_len + 1);
                    strncpy(number, str_start, str_len);
                    number[str_len] = '\0';

                    if (decimal_count == 1)
                        add_token(lexer, TOKEN_DECIMAL, number);
                    else
                        add_token(lexer, TOKEN_NUMBER, number);

                    continue;
                } else if (*lexer->cur_tok == '"') {
                    lexer->cur_tok++;
                    char *str_start = lexer->cur_tok;

                    while (*lexer->cur_tok != '"' && *lexer->cur_tok != '\0') {
                        lexer->cur_tok++;
                    }

                    if (*lexer->cur_tok == '\0') {
                        fprintf(stderr, "%zu: Error: Missing closing quote for string literal.\n", lexer->line_number);
                        return 1; 
                    }
                    

                    int str_len = lexer->cur_tok - str_start;
                    char *text = (char *)malloc(str_len + 1);
                    
                    strncpy(text, str_start, str_len);
                    text[str_len] = '\0';
                    add_token(lexer, TOKEN_WORD, text);
                    lexer->cur_tok++;
                    continue;

                } else if (isalpha(*lexer->cur_tok) || *lexer->cur_tok == '_') {
                    handle_word(lexer);
                    continue;

                } else {
                    fprintf(stderr, "Unknown token: %c\n", *lexer->cur_tok);
                    return 1;
                }
                break;

         }
        // advance token
        lexer->cur_tok++;
    }
    add_token(lexer, TOKEN_EOF, NULL);
    return 0;
}

static void handle_identifier(Lexer *lexer) {
    char *str_start = lexer->cur_tok;

    // everything except the first character can be a number
    while (isalnum(*lexer->cur_tok)) lexer->cur_tok++;

    int str_len = lexer->cur_tok - str_start;
    Token keyword_token = handle_keyword(str_start, str_len);
    
    char *identifier = (char *)malloc(str_len + 1);

    strncpy(identifier, str_start, str_len);
    identifier[str_len] = '\0';

    
    if (keyword_token != TOKEN_IDENTIFIER) {
        const char *keyword_text = get_keyword_static_text(keyword_token);
        add_token(lexer, keyword_token, identifier);
    } else {
        add_token(lexer, TOKEN_IDENTIFIER, identifier);
    }
}

Token handle_keyword(const char *start, size_t length) {
    // Iterate through the array of reserved words
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        const Keyword *keyword = &KEYWORDS[i];
        
        // 1. Check if lengths match (quick optimization)
        if (length != strlen(keyword->text)) {
            continue;
        }

        // 2. Perform a full string comparison
        if (strncmp(start, keyword->text, length) == 0) {
            // Match found! Return the reserved token type.
            return keyword->type;
        }
    }

    // No match found in the reserved list, so it's a regular identifier.
    return TOKEN_IDENTIFIER;
}

const char* get_keyword_static_text(Token type) {
    
    // Iterate through the entire list of reserved words
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        // Check if the input token type matches the one in the array
        if (KEYWORDS[i].type == type) {
            // Match found: Return the associated static string literal
            return KEYWORDS[i].text;
        }
    }

    // Should theoretically not happen if the input 'type' came from a successful keyword check
    return NULL; 
}

// Free the memory allocated for the lexer
void free_lexer(Lexer *lexer) {

    for (size_t i = 0; i < lexer->token_count; i++) {
        
        if (lexer->tokens[i].type == TOKEN_IDENTIFIER || 
            lexer->tokens[i].type == TOKEN_WORD ||
            lexer->tokens[i].type == TOKEN_NUMBER ||
            lexer->tokens[i].type == TOKEN_DECIMAL ||
            lexer->tokens[i].type == TOKEN_LETTER) 
        {
            free(lexer->tokens[i].val);
        }
    }
    
    // 3. Free the main array holding the TokenData structs
    free(lexer->tokens);
}

void printLexerTokens(const Lexer *lexer) {
    
    // Print the header that matches the desired format
    printf("\n\n--------------------------------\n");
    printf("| %-20s | %-15s |\n", "Lexeme", "Token");
    printf("--------------------------------\n");

    // Iterate through all tokens in the array
    for (size_t i = 0; i < lexer->token_count; i++) {
        const TokenData *token = &lexer->tokens[i];
        
        // Get the printable name for the token type
        const char *token_name = getTokenName(token->type);

        // Print the lexeme and the token name
        printf("| %-20s | %-15s |\n", token->val, token_name);
        
        // Optional: Print location information if desired
        // printf(" (L:%zu, C:%zu)\n", token->loc.line, token->loc.col);
    }
    
    printf("--------------------------------\n");
}

// Array of all reserved words/operators
static const Keyword KEYWORDS[] = {
    {"greaterequal", TOKEN_GREATEREQUAL},
    {"lessequal",    TOKEN_LESSEQUAL},
    {"remainder",    TOKEN_REMAINDER},
    {"increase",     TOKEN_INCREASE},
    {"decrease",     TOKEN_DECREASE},
    {"greater",      TOKEN_GREATER},
    {"less",         TOKEN_LESS},
    {"scale",        TOKEN_SCALE},
    {"split",        TOKEN_SPLIT},
    {"plus",         TOKEN_PLUS},
    {"min",          TOKEN_MIN},
    {"mul",          TOKEN_MUL},
    {"div",          TOKEN_DIV},
    {"mod",          TOKEN_MOD},
    {"idiv",         TOKEN_IDIV},
    {"pow",          TOKEN_POW},
    {"pos",          TOKEN_POS},
    {"neg",          TOKEN_NEG},
    {"next",         TOKEN_NEXT},
    {"prev",         TOKEN_PREV},
    {"is",           TOKEN_IS},
    {"isnt",         TOKEN_ISNT},
    {"and",          TOKEN_AND},
    {"or",           TOKEN_OR},
    {"not",          TOKEN_NOT},
    {"set",          TOKEN_SET},
    {"start",        TOKEN_START},
    {"end",          TOKEN_END},
    {"continue",     TOKEN_CONTINUE},
    {"stop",         TOKEN_STOP},
    {"if",           TOKEN_IF},
    {"else",         TOKEN_ELSE},
    {"while",        TOKEN_WHILE},
    {"exit",         TOKEN_EXIT},
    {"loop",         TOKEN_LOOP},
    {"main",         TOKEN_MAIN},
    {"true",         TOKEN_TRUE},
    {"false",        TOKEN_FALSE}
};

// Calculate the number of entries in the array
static const size_t KEYWORD_COUNT = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);