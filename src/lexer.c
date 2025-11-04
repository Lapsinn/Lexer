#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
                    handle_identifier(lexer);
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

static void add_token(Lexer *lexer, Token type, char *val) {
    if (lexer->token_count == lexer->capacity) {
        size_t new_capacity = lexer->capacity == 0 ? 8 : lexer->capacity * 2;
        lexer->tokens = (TokenData *)realloc(lexer->tokens, new_capacity * sizeof(TokenData));
        if (!lexer->tokens) {
            perror("Error reallocating memory");
            exit(1);
        }

        lexer->capacity = new_capacity;
    }

    lexer->tokens[lexer->token_count].type = type;
    lexer->tokens[lexer->token_count].val = val;

    // setting the location
    lexer->tokens[lexer->token_count].loc.line = lexer->line_number;
    lexer->tokens[lexer->token_count].loc.col = lexer->cur_tok - lexer->line_start;

    lexer->token_count++;
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
        const char *token_name = token_type_to_string(token->type);

        // Print the lexeme and the token name
        printf("| %-20s | %-15s |\n", token->val, token_name);
    }
    
    printf("--------------------------------\n");
}

const char *token_type_to_string(Token type) {
    switch (type) {
        // Core Tokens
        case TOKEN_EOF:         return "TOKEN_EOF";
        case TOKEN_IDENTIFIER:  return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:      return "TOKEN_NUMBER";
        case TOKEN_DECIMAL:     return "TOKEN_DECIMAL";
        case TOKEN_LETTER:      return "TOKEN_LETTER";
        case TOKEN_WORD:        return "TOKEN_WORD";
        case TOKEN_TRUE:        return "TOKEN_TRUE";
        case TOKEN_FALSE:       return "TOKEN_FALSE";
        case TOKEN_SEMICOLON:   return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:       return "TOKEN_COMMA";
        
        // Brackets/Punctuation
        case TOKEN_LPAREN:      return "TOKEN_LPAREN";
        case TOKEN_RPAREN:      return "TOKEN_RPAREN";
        case TOKEN_LBRACE:      return "TOKEN_LBRACE";
        case TOKEN_RBRACE:      return "TOKEN_RBRACE";
        case TOKEN_LBRACKET:    return "TOKEN_LBRACKET";
        case TOKEN_RBRACKET:    return "TOKEN_RBRACKET";

 /*      // Keywords (Control Flow)
        case TOKEN_START:       return "TOKEN_START";
        case TOKEN_END:         return "TOKEN_END";
        case TOKEN_CONTINUE:    return "TOKEN_CONTINUE";
        case TOKEN_STOP:        return "TOKEN_STOP";
        case TOKEN_IF:          return "TOKEN_IF";
        case TOKEN_ELSE:        return "TOKEN_ELSE";
        case TOKEN_WHILE:       return "TOKEN_WHILE";
*/
        // Value Setting Operators
        case TOKEN_SET:         return "TOKEN_SET";
        case TOKEN_INCREASE:    return "TOKEN_INCREASE";
        case TOKEN_DECREASE:    return "TOKEN_DECREASE";
        case TOKEN_SCALE:       return "TOKEN_SCALE";
        case TOKEN_SPLIT:       return "TOKEN_SPLIT";
        case TOKEN_REMAINDER:   return "TOKEN_REMAINDER";
        
        // Arithmetic Operators
        case TOKEN_PLUS:        return "TOKEN_PLUS";
        case TOKEN_MIN:         return "TOKEN_MIN";
        case TOKEN_MUL:         return "TOKEN_MUL";
        case TOKEN_DIV:         return "TOKEN_DIV";
        case TOKEN_MOD:         return "TOKEN_MOD";
        case TOKEN_IDIV:        return "TOKEN_IDIV";
        case TOKEN_POW:         return "TOKEN_POW";
        
        // Single Value Operators (Unary)
        case TOKEN_POS:         return "TOKEN_POS";
        case TOKEN_NEG:         return "TOKEN_NEG";
        case TOKEN_NEXT:        return "TOKEN_NEXT";
        case TOKEN_PREV:        return "TOKEN_PREV";
        
        // Comparison Operators (Relational)
        case TOKEN_IS:          return "TOKEN_IS";
        case TOKEN_ISNT:        return "TOKEN_ISNT";
        case TOKEN_GREATER:     return "TOKEN_GREATER";
        case TOKEN_LESS:        return "TOKEN_LESS";
        case TOKEN_GREATEREQUAL: return "TOKEN_GREATEREQUAL";
        case TOKEN_LESSEQUAL:   return "TOKEN_LESSEQUAL";
        
        // Logic Operators
        case TOKEN_AND:         return "TOKEN_AND";
        case TOKEN_OR:          return "TOKEN_OR";
        case TOKEN_NOT:         return "TOKEN_NOT";
        
        // Reserved Words
        case TOKEN_EXIT:        return "TOKEN_EXIT";
        case TOKEN_LOOP:        return "TOKEN_LOOP";
        case TOKEN_MAIN:        return "TOKEN_MAIN";

        default:
            // This case handles any unlisted or unknown token types, 
            // preventing the function from returning garbage.
            return "TOKEN_UNKNOWN";
    }
}