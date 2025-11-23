#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CHAR '\0'

int lex(Lexer *lexer) {
    while (*lexer->cur_tok != '\0') {
      //  printf("Current char: '%c'\n", *lexer->cur_tok);
        switch (*lexer->cur_tok) {
            case ' ':
            lexer->cur_tok++;
            continue;
            
        case '\n':
            lexer->line_number++;
            lexer->line_start = lexer->cur_tok + 1;
            lexer->cur_tok++; 
            continue;

        case '(':
            add_token(lexer, TOKEN_LPAREN, "(", 0);
            lexer->cur_tok++;
            continue; 

        case ')':
            add_token(lexer, TOKEN_RPAREN, ")", 0);
            lexer->cur_tok++;
            continue; 

        case '{':
            add_token(lexer, TOKEN_LBRACE, "{", 0);
            lexer->cur_tok++;
            continue; 

        case '}':
            add_token(lexer, TOKEN_RBRACE, "}", 0);
            lexer->cur_tok++;
            continue; 

        case '[':
            add_token(lexer, TOKEN_LBRACKET, "[", 0);
            lexer->cur_tok++;
            continue; 

        case ']':
            add_token(lexer, TOKEN_RBRACKET, "]", 0);
            lexer->cur_tok++;
            continue; 

        case ',':
            add_token(lexer, TOKEN_COMMA, ",", 0);
            lexer->cur_tok++;
            continue; 

        case ';':
            add_token(lexer, TOKEN_SEMICOLON, ";", 0);
            lexer->cur_tok++;
            continue; 

        case '#':
            while (*lexer->cur_tok != '\n' && *lexer->cur_tok != '\0') lexer->cur_tok++;
            continue; 

        case ':':
            add_token(lexer, TOKEN_COLON, ":", 0);
            lexer->cur_tok++;
            continue; 

        case '.':
            add_token(lexer, TOKEN_DOT, ".", 0);
            lexer->cur_tok++;
            continue;

        case '+':
            add_token(lexer, TOKEN_PLUS, "+", 0);
            lexer->cur_tok++;
            continue; 

        case '-':
            add_token(lexer, TOKEN_MIN, "-", 0); 
            lexer->cur_tok++;
            continue; 

        case '*':
            add_token(lexer, TOKEN_MUL, "*", 0);
            lexer->cur_tok++;
            continue;

        case '/':
            add_token(lexer, TOKEN_DIV, "/", 0);
            lexer->cur_tok++;
            continue;

        case '%':
            add_token(lexer, TOKEN_MOD, "%", 0);
            lexer->cur_tok++;
            continue;

        case '~':
            add_token(lexer, TOKEN_IDIV, "~", 0);
            lexer->cur_tok++;
            continue;

        case '^':
            add_token(lexer, TOKEN_POW,  "^", 0);
            lexer->cur_tok++;
            continue;

        case '\\':
            add_token(lexer, INVALID, "\\", 0);
            lexer->cur_tok++;
            continue;

        case '>': {
            if (*(lexer->cur_tok + 1) == '=') {
                add_token(lexer, TOKEN_GREATEREQUAL, ">=", 0); 
                lexer->cur_tok += 2;
            } else {
                add_token(lexer, TOKEN_GREATER, ">", 0);
                lexer->cur_tok++;
            }
            continue;
        }

        case '<': {
            if (*(lexer->cur_tok + 1) == '=') {
                add_token(lexer, TOKEN_LESSEQUAL,  "<=", 0); 
                lexer->cur_tok += 2; 
            } else {
                add_token(lexer, TOKEN_LESS, "<", 0);
                lexer->cur_tok++;
            }
            continue;
        }

        case '=': {
            if (*(lexer->cur_tok + 1) == '=') {
                add_token(lexer, TOKEN_IS, "==", 0); 
                lexer->cur_tok += 2;
            } else {
                add_token(lexer, TOKEN_ASSIGN, "=", 0);
                lexer->cur_tok++;
            }
            continue;
        }

        case '!': {
            if (*(lexer->cur_tok + 1) == '=') {
                add_token(lexer, TOKEN_ISNT, "!=", 0); 
                lexer->cur_tok += 2; 
            } else {
                add_token(lexer, TOKEN_NOT, "!", 0);
                lexer->cur_tok++;
            }
            continue;
        }

        case '&': {
            if (*(lexer->cur_tok + 1) == '&') {
                add_token(lexer, TOKEN_AND, "&&", 0); 
                lexer->cur_tok += 2;
            } else {
                fprintf(stderr, "%zu: Error: Unexpected character '&'\n", lexer->line_number);
                add_token(lexer, INVALID, "&", 0);
                lexer->cur_tok++;
            }
            continue;
        }

        case '|': {
            if (*(lexer->cur_tok + 1) == '|') {
                add_token(lexer, TOKEN_OR, "||", 0); 
                lexer->cur_tok += 2;
            } else {
                fprintf(stderr, "%zu: Error: Unexpected character '|'\n", lexer->line_number);
                add_token(lexer, INVALID, "|", 0);
                lexer->cur_tok++;
            }
            continue;
        }

        case '"': {
            lexer->cur_tok++;
            char *str_start = lexer->cur_tok;
            Token token_type = TOKEN_STRING_LITERAL;

            while (*lexer->cur_tok != '"' && *lexer->cur_tok != '\0') {
                lexer->cur_tok++;
            }

            if (*lexer->cur_tok == '\0') {
                fprintf(stderr, "%zu: Error: Missing closing quote for string literal.\n", lexer->line_number);
                token_type = INVALID;
            }
            

            int str_len = lexer->cur_tok - str_start;
            char *text = (char *)malloc(str_len + 1);
            if (text == NULL) {
                fprintf(stderr, "Fatal Error: Memory allocation failed for string literal\n");
                exit(1);
            }
            
            strncpy(text, str_start, str_len);
            text[str_len] = '\0';
            add_token(lexer, token_type, text, 1);
            lexer->cur_tok++;
            continue;
        }

        case '\'': {
            lexer->cur_tok++;
            char *char_start = lexer->cur_tok;
            Token token_type = TOKEN_CHAR_LITERAL;
            
            if (*lexer->cur_tok == '\0' || *lexer->cur_tok == '\n') {
                fprintf(stderr, "%zu: Error: Unterminated character literal.\n", lexer->line_number);
                // Do not advance `cur_tok` past '\0' or '\n' here.
                add_token(lexer, INVALID, "'", 0);
                continue;
            }
            
            char value = *lexer->cur_tok;
            lexer->cur_tok++;
            
            if (*lexer->cur_tok != '\'') {
                while (*lexer->cur_tok != '\'' && *lexer->cur_tok != '\0' && *lexer->cur_tok != '\n') {
                    lexer->cur_tok++;
                }
                
                fprintf(stderr, "%zu: Error: Invalid character literal (expected exactly one character).\n", lexer->line_number);
                token_type = INVALID;

                if (*lexer->cur_tok == '\'') {
                    lexer->cur_tok++; // Consume the closing quote
                }
            } else {
                lexer->cur_tok++; // Consume the closing quote
            }

            char *char_str = (char *)malloc(2);
            if (char_str == NULL) {
                fprintf(stderr, "Fatal Error: Memory allocation failed for character literal\n");
                exit(1);
            }
            char_str[0] = value;
            char_str[1] = '\0';
            
            add_token(lexer, token_type, char_str, 1);
            free(char_str);
            continue;
        }
        default: 
            if (isalpha(*lexer->cur_tok) || *lexer->cur_tok == '_') {
                handle_identifier(lexer);
                continue;
            } else if (isdigit(*lexer->cur_tok) || (*lexer->cur_tok == '.' && isdigit(*(lexer->cur_tok + 1)))) {
                handle_number_token(lexer);
                continue; 
            } else {
                fprintf(stderr, "Unknown token at line %zu, col %zu: '%c' (ASCII: %d)\n", 
                        lexer->line_number, 
                        lexer->cur_tok - lexer->line_start,
                        *lexer->cur_tok,
                        (int)*lexer->cur_tok);
                add_token(lexer, INVALID, lexer->cur_tok, 0);
            }
            break;
         }
    }
    add_token(lexer, TOKEN_EOF, NULL, 0);
    return 0;
}

static void add_token(Lexer *lexer, Token type, char *val, int malloced) {
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
    lexer->tokens[lexer->token_count].need_free = malloced;

    // setting the location
    lexer->tokens[lexer->token_count].loc.line = lexer->line_number;
    lexer->tokens[lexer->token_count].loc.col = lexer->cur_tok - lexer->line_start;

    lexer->token_count++;
}

static void handle_identifier(Lexer *lexer) {
    char *str_start = lexer->cur_tok;

    // everything except the first character can be a number
    if(*str_start == '_') {
        lexer->cur_tok++;
    }

    while (isalnum(*lexer->cur_tok) || *lexer->cur_tok == '_') {
    //    printf("Current char in identifier: '%c'\n", *lexer->cur_tok); // Debugging line
        lexer->cur_tok++;
    } 

    char *last_consumed_char_ptr = lexer->cur_tok - 1;
    if (*last_consumed_char_ptr == '_') {
        // Truncate the word: leave the invalid '_' for the next token.
        lexer->cur_tok--; 
    }

    int str_len = lexer->cur_tok - str_start;
    Token keyword_token = handle_keyword(str_start, str_len);
    
    char *identifier = (char *)malloc(str_len + 1);
    if (identifier == NULL) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for identifier\n");
        exit(1);
    }

    strncpy(identifier, str_start, str_len);
    identifier[str_len] = '\0';
    str_to_lower(identifier);

    //printf("Identified identifier: '%s'\n", identifier); // Debugging line
    if (keyword_token != TOKEN_IDENTIFIER) {

        add_token(lexer, keyword_token, identifier, 1);
    } else {
        add_token(lexer, TOKEN_IDENTIFIER, identifier, 1);
    }
}

void handle_number_token(Lexer *lexer) {
    char *str_start = lexer->cur_tok;
    int decimal_count = 0;
    int is_valid = 1; 
    
    if (*str_start == '.') {
        // Must be followed by a digit (e.g., .123)
        if (!isdigit(*(str_start + 1))) {
            return;
        }
        decimal_count = 1;
    }
    
 
    while (isdigit(*lexer->cur_tok) || *lexer->cur_tok == '.') {
        if (*lexer->cur_tok == '.') {
            if (++decimal_count > 1) {
                // We've found the second decimal point.
                is_valid = 0; 
                fprintf(stderr, "%zu: Error: Invalid number format — multiple decimal points\n", 
                                 lexer->line_number);
                break; 
            }
        }
        lexer->cur_tok++;
    }

    //Recovery
    if (!is_valid) {
        // CRITICAL: Consume the remaining garbage as part of the INVALID token's lexeme
        while (isalnum(*lexer->cur_tok) || *lexer->cur_tok == '.') {
            lexer->cur_tok++;
        }
    }

    int str_len = lexer->cur_tok - str_start;
    char *number_str = (char *)malloc(str_len + 1);
    if (number_str == NULL) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for number token\n");
        exit(1);
    }
    
    strncpy(number_str, str_start, str_len);
    number_str[str_len] = '\0';

    // Decide the token type based on the flag
    Token token_type;
    if (!is_valid) {
        token_type = INVALID;
    } else if (decimal_count >= 1) {
        token_type = TOKEN_DECIMAL; // Floating point number
    } else {
        token_type = TOKEN_NUMBER; // Integer number
    }

    add_token(lexer, token_type, number_str, 1);
}

Token handle_keyword(const char *input_word, size_t word_length) {
    
    State current_state = S_START;
    const StateNode* current_node = get_node(S_START);

    for (size_t i = 0; i < word_length; i++) {
        char c = tolower((unsigned char)input_word[i]);

        if (c < 'a' || c > 'z') {
            return TOKEN_IDENTIFIER; 
        }

        if (current_state == S_IDENT) {
            continue; 
        }

        State next_state = S_IDENT; // Initialize

        for (int j = 0; j < current_node->num_transitions; j++) {
            char transition_char = current_node->transitions[j].input_char;

            // Check for specific character match
            if (transition_char == c) {
                next_state = current_node->transitions[j].next_state;
                break;
            }


            if (transition_char == DEFAULT_CHAR) {
                next_state = current_node->transitions[j].next_state;
            }
        }
        // Move to the next state
        current_state = next_state; 
        current_node = get_node(current_state);
    }

    // Final check for an accepting state
    if (current_node->output != TOKEN_NONE) {
        return current_node->output;
    }
    return TOKEN_IDENTIFIER;
}

static void str_to_lower(char *str) {
    for (char *p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
}

// Free the memory allocated for the lexer
void free_lexer(Lexer *lexer) {

    for (size_t i = 0; i < lexer->token_count; i++) {
        if (lexer->tokens[i].need_free) {
            free(lexer->tokens[i].val);
        }
    }
    free(lexer->tokens);
}

void printLexerTokens(const Lexer *lexer) {
    
    printf("\n\n--------------------------------\n");
    printf("| %-20s | %-15s |\n", "Lexeme", "Token");
    printf("--------------------------------\n");

 
    for (size_t i = 0; i < lexer->token_count; i++) {
        const TokenData *token = &lexer->tokens[i];
        
        // Get the printable name for the token type
        const char *token_name = token_type_to_string(token->type);

        // Print the lexeme and the token name
        printf("| %-20s | %-15s |\n", token->val, token_name);
    }
    
    printf("--------------------------------\n");
}


const StateNode MACHINE_DEF[NUM_STATES] = {
    [S_START] = {S_START, TOKEN_NONE, {
        {'a', S_A}, {'o', S_O}, {'n', S_N}, {'s', S_S}, {'e', S_E}, 
        {'c', S_C}, {'i', S_I}, {'w', S_W}, {'l', S_L}, {'m', S_M},
        {'t', S_T}, {'f', S_F}, {'r', S_R}, {'d', S_D}, {'g', S_G},
        {DEFAULT_CHAR, S_IDENT} // Explicit Default Transition
    }, 17}, 

    [S_A] = {S_A, TOKEN_NONE, {{'n', S_AN}, {'s', S_AS}, {'l', S_AL}, {DEFAULT_CHAR, S_IDENT}}, 4},
    [S_AN] = {S_AN, TOKEN_NONE, {{'d', S_AND}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_AND] = {S_AND, TOKEN_AND, {{DEFAULT_CHAR, S_IDENT}}, 1}, 
    [S_AS] = {S_AS, TOKEN_NONE, {{'k', S_ASK}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_ASK] = {S_ASK, TOKEN_ASK, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_AL] = {S_AL, TOKEN_NONE, {{'s', S_ALS}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_ALS] = {S_ALS, TOKEN_NONE, {{'o', S_ALSO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_ALSO] = {S_ALSO, TOKEN_ALSO, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_C] = {S_C, TOKEN_NONE, {{'o', S_CO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CO] = {S_CO, TOKEN_NONE, {{'n', S_CON}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CON] = {S_CON, TOKEN_NONE, {{'t', S_CONT}, {'s', S_CONS}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_CONS] = {S_CONS, TOKEN_NONE, {{'t', S_CONST}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CONST] = {S_CONST, TOKEN_CONST, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_CONT] = {S_CONT, TOKEN_NONE, {{'i', S_CONTI}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CONTI] = {S_CONTI, TOKEN_NONE, {{'n', S_CONTIN}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CONTIN] = {S_CONTIN, TOKEN_NONE, {{'u', S_CONTINU}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CONTINU] = {S_CONTINU, TOKEN_NONE, {{'e', S_CONTINUE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_CONTINUE] = {S_CONTINUE, TOKEN_CONTINUE, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_D] = {S_D, TOKEN_NONE, {{'e', S_DE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_DE] = {S_DE, TOKEN_NONE, {{'c', S_DEC}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_DEC] = {S_DEC, TOKEN_NONE, {{'i', S_DECI}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_DECI] = {S_DECI, TOKEN_NONE, {{'m', S_DECIM}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_DECIM] = {S_DECIM, TOKEN_NONE, {{'a', S_DECIMA}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_DECIMA] = {S_DECIMA, TOKEN_NONE, {{'l', S_DECIMAL}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_DECIMAL] = {S_DECIMAL, TOKEN_DECIMAL, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_E] = {S_E, TOKEN_NONE, {{'n', S_EN}, {'l', S_EL}, {'x', S_EX}, {'a', S_EA}, {DEFAULT_CHAR, S_IDENT}}, 5},
    [S_EN] = {S_EN, TOKEN_NONE, {{'d', S_END}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_END] = {S_END, TOKEN_END, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_EL] = {S_EL, TOKEN_NONE, {{'s', S_ELS}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_ELS] = {S_ELS, TOKEN_NONE, {{'e', S_ELSE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_ELSE] = {S_ELSE, TOKEN_ELSE, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_EX] = {S_EX, TOKEN_NONE, {{'i', S_EXI}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_EXI] = {S_EXI, TOKEN_NONE, {{'t', S_EXIT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_EXIT] = {S_EXIT, TOKEN_EXIT, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_EA] = {S_EA, TOKEN_NONE, {{'c', S_EAC}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_EAC] = {S_EAC, TOKEN_NONE, {{'h', S_EACH}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_EACH] = {S_EACH, TOKEN_EACH, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_F] = {S_F, TOKEN_NONE, {{'a', S_FA}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_FA] = {S_FA, TOKEN_NONE, {{'l', S_FAL}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_FAL] = {S_FAL, TOKEN_NONE, {{'s', S_FALS}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_FALS] = {S_FALS, TOKEN_NONE, {{'e', S_FALSE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_FALSE] = {S_FALSE, TOKEN_FALSE, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_G] = {S_G, TOKEN_NONE, {{'r', S_GR}, {'o', S_GO}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_GO] = {S_GO, TOKEN_NONE, {{'t', S_GOT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GOT] = {S_GOT, TOKEN_NONE, {{'o', S_GOTO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GOTO] = {S_GOTO, TOKEN_GOTO, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_GR] = {S_GR, TOKEN_NONE, {{'e', S_GRE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GRE] = {S_GRE, TOKEN_NONE, {{'a', S_GREA}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GREA] = {S_GREA, TOKEN_NONE, {{'t', S_GREAT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GREAT] = {S_GREAT, TOKEN_NONE, {{'e', S_GREATE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GREATE] = {S_GREATE, TOKEN_NONE, {{'r', S_GREATER}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_GREATER] = {S_GREATER, TOKEN_GREATER, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_I] = {S_I, TOKEN_NONE, {{'f', S_IF}, {'m', S_IM}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_IF] = {S_IF, TOKEN_IF, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_IM] = {S_IM, TOKEN_NONE, {{'p', S_IMP}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_IMP] = {S_IMP, TOKEN_NONE, {{'o', S_IMPO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_IMPO] = {S_IMPO, TOKEN_NONE, {{'r', S_IMPOR}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_IMPOR] = {S_IMPOR, TOKEN_NONE, {{'t', S_IMPORT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_IMPORT] = {S_IMPORT, TOKEN_IMPORT, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_L] = {S_L, TOKEN_NONE, {{'o', S_LO}, {'e', S_LE}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_LO] = {S_LO, TOKEN_NONE, {{'o', S_LOO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_LOO] = {S_LOO, TOKEN_NONE, {{'p', S_LOOP}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_LOOP] = {S_LOOP, TOKEN_LOOP, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_LE] = {S_LE, TOKEN_NONE, {{'t', S_LET}, {'s', S_LES}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_LET] = {S_LET, TOKEN_NONE, {{'t', S_LETT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_LETT] = {S_LETT, TOKEN_NONE, {{'e', S_LETTE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_LETTE] = {S_LETTE, TOKEN_NONE, {{'r', S_LETTER}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_LETTER] = {S_LETTER, TOKEN_LETTER, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_LES] = {S_LES, TOKEN_NONE, {{'s', S_LESS}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_LESS] = {S_LESS, TOKEN_LESS, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_M] = {S_M, TOKEN_NONE, {{'a', S_MA}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_MA] = {S_MA, TOKEN_NONE, {{'i', S_MAI}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_MAI] = {S_MAI, TOKEN_NONE, {{'n', S_MAIN}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_MAIN] = {S_MAIN, TOKEN_MAIN, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_N] = {S_N, TOKEN_NONE, {{'o', S_NO}, {'u', S_NU}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_NO] = {S_NO, TOKEN_NONE, {{'t', S_NOT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_NOT] = {S_NOT, TOKEN_NOT, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_NU] = {S_NU, TOKEN_NONE, {{'m', S_NUM}, {'l', S_NUL}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_NUL] = {S_NUL, TOKEN_NONE, {{'l', S_NULL}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_NULL] = {S_NULL, TOKEN_NULL, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_NUM] = {S_NUM, TOKEN_NONE, {{'b', S_NUMB}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_NUMB] = {S_NUMB, TOKEN_NONE, {{'e', S_NUMBE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_NUMBE] = {S_NUMBE, TOKEN_NONE, {{'r', S_NUMBER}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_NUMBER] = {S_NUMBER, TOKEN_NUMBER, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_O] = {S_O, TOKEN_NONE, {{'r', S_OR}, {'f', S_OF}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_OR] = {S_OR, TOKEN_OR, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_OF] = {S_OF, TOKEN_OF, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_R] = {S_R, TOKEN_NONE, {{'e', S_RE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_RE] = {S_RE, TOKEN_NONE, {{'p', S_REP}, {'t', S_RET}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_REP] = {S_REP, TOKEN_NONE, {{'e', S_REPE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_REPE] = {S_REPE, TOKEN_NONE, {{'a', S_REPEA}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_REPEA] = {S_REPEA, TOKEN_NONE, {{'t', S_REPEAT}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_REPEAT] = {S_REPEAT, TOKEN_REPEAT, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_RET] = {S_RET, TOKEN_NONE, {{'u', S_RETU}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_RETU] = {S_RETU, TOKEN_NONE, {{'r', S_RETUR}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_RETUR] = {S_RETUR, TOKEN_NONE, {{'n', S_RETURN}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_RETURN] = {S_RETURN, TOKEN_RETURN, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_S] = {S_S, TOKEN_NONE, {{'t', S_ST}, {'h', S_SH}, {'i', S_SI}, {DEFAULT_CHAR, S_IDENT}}, 4},
    [S_SI] = {S_SI, TOKEN_NONE, {{'z', S_SIZ}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_SIZ] = {S_SIZ, TOKEN_NONE, {{'e', S_SIZE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_SIZE] = {S_SIZE, TOKEN_NONE, {{'o', S_SIZEO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_SIZEO] = {S_SIZEO, TOKEN_NONE, {{'f', S_SIZEOF}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_SIZEOF] = {S_SIZEOF, TOKEN_SIZEOF, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_ST] = {S_ST, TOKEN_NONE, {{'a', S_STA}, {'o', S_STO}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_STA] = {S_STA, TOKEN_NONE, {{'r', S_STAR}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_STAR] = {S_STAR, TOKEN_NONE, {{'t', S_START_KW}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_START_KW] = {S_START_KW, TOKEN_START, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_STO] = {S_STO, TOKEN_NONE, {{'p', S_STOP}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_STOP] = {S_STOP, TOKEN_STOP, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_SH] = {S_SH, TOKEN_NONE, {{'o', S_SHO}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_SHO] = {S_SHO, TOKEN_NONE, {{'w', S_SHOW}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_SHOW] = {S_SHOW, TOKEN_SHOW, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_T] = {S_T, TOKEN_NONE, {{'r', S_TR}, {'h', S_TH}, {DEFAULT_CHAR, S_IDENT}}, 4},
    [S_TR] = {S_TR, TOKEN_NONE, {{'u', S_TRU}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_TRU] = {S_TRU, TOKEN_NONE, {{'e', S_TRUE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_TRUE] = {S_TRUE, TOKEN_TRUE, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_TH] = {S_TH, TOKEN_NONE, {{'e', S_THE}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_THE] = {S_THE, TOKEN_NONE, {{'n', S_THEN}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_THEN] = {S_THEN, TOKEN_THEN, {{DEFAULT_CHAR, S_IDENT}}, 1},

    [S_W] = {S_W, TOKEN_NONE, {{'h', S_WH}, {'o', S_WO}, {DEFAULT_CHAR, S_IDENT}}, 3},
    [S_WH] = {S_WH, TOKEN_NONE, {{'i', S_WHI}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_WHI] = {S_WHI, TOKEN_NONE, {{'l', S_WHIL}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_WHIL] = {S_WHIL, TOKEN_NONE, {{'e', S_WHILE}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_WHILE] = {S_WHILE, TOKEN_WHILE, {{DEFAULT_CHAR, S_IDENT}}, 1},
    [S_WO] = {S_WO, TOKEN_NONE, {{'r', S_WOR}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_WOR] = {S_WOR, TOKEN_NONE, {{'d', S_WORD}, {DEFAULT_CHAR, S_IDENT}}, 2},
    [S_WORD] = {S_WORD, TOKEN_WORD, {{DEFAULT_CHAR, S_IDENT}}, 1},

    // The Dead State definition
    [S_IDENT] = {S_IDENT, TOKEN_NONE, {
        {DEFAULT_CHAR, S_IDENT} 
    }, 1}
};

const StateNode* get_node(State s) {
    if (s >= 0 && s < NUM_STATES) {
        return &MACHINE_DEF[s];
    }
    return NULL; 
}


const char *token_type_to_string(Token type) {
    switch (type) {
        case TOKEN_IDENTIFIER:      return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:          return "TOKEN_NUMBER";
        case TOKEN_DECIMAL:         return "TOKEN_DECIMAL";
        case TOKEN_LETTER:          return "TOKEN_LETTER";
        case TOKEN_WORD:            return "TOKEN_WORD";
        case TOKEN_TRUE:            return "TOKEN_TRUE";
        case TOKEN_FALSE:           return "TOKEN_FALSE";
        case TOKEN_SEMICOLON:       return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:           return "TOKEN_COMMA";
        case TOKEN_CONST:           return "TOKEN_CONST";
        case TOKEN_NULL:            return "TOKEN_NULL";
        case TOKEN_SIZEOF:          return "TOKEN_SIZEOF";
        case TOKEN_RETURN:          return "TOKEN_RETURN";
     
        case TOKEN_STRING_LITERAL:  return "TOKEN_STRING_LITERAL";
        case TOKEN_CHAR_LITERAL:    return "TOKEN_CHAR_LITERAL";
        case TOKEN_COLON:           return "TOKEN_COLON";
        case TOKEN_DOT:             return "TOKEN_DOT";
      
        case TOKEN_LPAREN:          return "TOKEN_LPAREN";
        case TOKEN_RPAREN:          return "TOKEN_RPAREN";
        case TOKEN_LBRACE:          return "TOKEN_LBRACE";
        case TOKEN_RBRACE:          return "TOKEN_RBRACE";
        case TOKEN_LBRACKET:        return "TOKEN_LBRACKET";
        case TOKEN_RBRACKET:        return "TOKEN_RBRACKET";

        case TOKEN_START:           return "TOKEN_START";
        case TOKEN_END:             return "TOKEN_END";
        case TOKEN_CONTINUE:        return "TOKEN_CONTINUE";
        case TOKEN_STOP:            return "TOKEN_STOP";
        case TOKEN_IF:              return "TOKEN_IF";
        case TOKEN_ELSE:            return "TOKEN_ELSE";
        case TOKEN_WHILE:           return "TOKEN_WHILE";

        case TOKEN_ASK:             return "TOKEN_ASK";
        case TOKEN_REPEAT:          return "TOKEN_REPEAT";
        case TOKEN_SHOW:            return "TOKEN_SHOW";
        
        case TOKEN_EACH:            return "TOKEN_EACH";
        case TOKEN_OF:              return "TOKEN_OF";
        case TOKEN_THEN:            return "TOKEN_THEN";
        case TOKEN_ALSO:            return "TOKEN_ALSO";

        case TOKEN_ASSIGN:          return "TOKEN_ASSIGN";
        case TOKEN_INCREASE:        return "TOKEN_INCREASE";
        case TOKEN_DECREASE:        return "TOKEN_DECREASE";
        case TOKEN_SCALE:           return "TOKEN_SCALE";
        case TOKEN_SPLIT:           return "TOKEN_SPLIT";
        case TOKEN_REMAINDER:       return "TOKEN_REMAINDER";
        
        case TOKEN_PLUS:            return "TOKEN_PLUS";
        case TOKEN_MIN:             return "TOKEN_MIN";
        case TOKEN_MUL:             return "TOKEN_MUL";
        case TOKEN_DIV:             return "TOKEN_DIV";
        case TOKEN_MOD:             return "TOKEN_MOD";
        case TOKEN_IDIV:            return "TOKEN_IDIV";
        case TOKEN_POW:             return "TOKEN_POW";
        
        case TOKEN_POS:             return "TOKEN_POS";
        case TOKEN_NEG:             return "TOKEN_NEG";
        case TOKEN_NEXT:            return "TOKEN_NEXT";
        case TOKEN_PREV:            return "TOKEN_PREV";
        
        case TOKEN_IS:              return "TOKEN_IS";
        case TOKEN_ISNT:            return "TOKEN_ISNT";
        case TOKEN_GREATER:         return "TOKEN_GREATER";
        case TOKEN_LESS:            return "TOKEN_LESS";
        case TOKEN_GREATEREQUAL:    return "TOKEN_GREATEREQUAL";
        case TOKEN_LESSEQUAL:       return "TOKEN_LESSEQUAL";
    
        case TOKEN_AND:             return "TOKEN_AND";
        case TOKEN_OR:              return "TOKEN_OR";
        case TOKEN_NOT:             return "TOKEN_NOT";
        
        case TOKEN_IMPORT:          return "TOKEN_IMPORT";
        case TOKEN_GOTO:            return "TOKEN_GOTO";
        case TOKEN_EXIT:            return "TOKEN_EXIT";
        case TOKEN_LOOP:            return "TOKEN_LOOP";
        case TOKEN_MAIN:            return "TOKEN_MAIN";
        case TOKEN_NONE:            return "TOKEN_NONE";
        case TOKEN_EOF:             return "TOKEN_EOF";
        case INVALID:               return "INVALID";
        
        default:
            return "TOKEN_UNKNOWN";
    }
}