#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lex(Lexer *lexer) {
    while (*lexer->cur_tok != '\0') {
      //  printf("Current char: '%c'\n", *lexer->cur_tok);
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
                break;
            case ':':
                add_token(lexer, TOKEN_COLON, ":");
                break;
            case '.':
                add_token(lexer, TOKEN_DOT, ".");
                break;
            case '+':
                add_token(lexer, TOKEN_PLUS, "+");
                break;
            case '-':
                add_token(lexer, TOKEN_MIN, "-"); 
                break;
            case '*':
                add_token(lexer, TOKEN_MUL, "*");
                break;
            case '/':
                add_token(lexer, TOKEN_DIV, "/");
                break;
            case '%':
                add_token(lexer, TOKEN_MOD, "%");
                break;
            case '~':
                add_token(lexer, TOKEN_IDIV, "~");
                break;
            case '^':
                add_token(lexer, TOKEN_POW, "^");
                break;
            case '\\':
                add_token(lexer, TOKEN_NONE, "\\");
                break;
            case '>': {
                if (*(lexer->cur_tok + 1) == '=') {
                    lexer->cur_tok++; 
                    add_token(lexer, TOKEN_GREATEREQUAL, ">="); 
                } else {
                    add_token(lexer, TOKEN_GREATER, ">");
                }
                lexer->cur_tok++;
                break;
            }

            case '<': {
                if (*(lexer->cur_tok + 1) == '=') {
                    lexer->cur_tok++; 
                    add_token(lexer, TOKEN_LESSEQUAL, "<="); 
                } else {

                    add_token(lexer, TOKEN_LESS, "<");
                }
                lexer->cur_tok++; 
                break;
            }

            case '=': {
                if (*(lexer->cur_tok + 1) == '=') {
                    lexer->cur_tok++; 
                    add_token(lexer, TOKEN_IS, "=="); 
                } else {
                    add_token(lexer, TOKEN_SET, "=");
                }
                lexer->cur_tok++;
                break;
            }

            case '!': {
                if (*(lexer->cur_tok + 1) == '=') {
                    lexer->cur_tok++; 
                    add_token(lexer, TOKEN_ISNT, "!="); 
                } else {
                    add_token(lexer, TOKEN_NOT, "!");
                }
                lexer->cur_tok++; 
                break;
            }
            case '&': {
                if (*(lexer->cur_tok + 1) == '&') {
                    lexer->cur_tok++; 
                    add_token(lexer, TOKEN_AND, "&&"); 
                } else {
                    fprintf(stderr, "%zu: Error: Unexpected character '&'\n", lexer->line_number);
                    return 1; 
                }
                lexer->cur_tok++; 
                break;
            }
            case '|': {
                if (*(lexer->cur_tok + 1) == '|') {
                    lexer->cur_tok++; 
                    add_token(lexer, TOKEN_OR, "||"); 
                } else {
                    fprintf(stderr, "%zu: Error: Unexpected character '|'\n", lexer->line_number);
                    return 1; 
                }
                lexer->cur_tok++; 
                break;
            }
            default:
                if (*lexer->cur_tok == '"') {
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
                    add_token(lexer, TOKEN_STRING_LITERAL, text);
                    lexer->cur_tok++;
                    continue;

                } else if (*lexer->cur_tok == '\'') {
                    lexer->cur_tok++;
                    char *char_start = lexer->cur_tok;
                    
                    if (*lexer->cur_tok == '\0') {
                        fprintf(stderr, "%zu: Error: Unterminated character literal.\n", lexer->line_number);
                        return 1; 
                    }
                    
                    char value = *lexer->cur_tok;
                    lexer->cur_tok++;
                    
                    if (*lexer->cur_tok != '\'') {
                        fprintf(stderr, "%zu: Error: Missing closing quote for character literal.\n", lexer->line_number);
                        return 1; 
                    }
                    
                    char *char_str = (char *)malloc(2);
                    char_str[0] = value;
                    char_str[1] = '\0';
                    add_token(lexer, TOKEN_CHAR_LITERAL, char_str);
                    lexer->cur_tok++; // Move past the closing quote
                    continue;

                } else if (isalpha(*lexer->cur_tok) || *lexer->cur_tok == '_') {
                    handle_identifier(lexer);
                    continue;
                } else if (isdigit(*lexer->cur_tok) || (*lexer->cur_tok == '.' && isdigit(*(lexer->cur_tok + 1)))) {
                    if (handle_number_token(lexer) != 0) {
                        return 1; 
                        }
                    continue; 
                } else {
                    fprintf(stderr, "Unknown token: %c\n", *(lexer->cur_tok-3));
                    fprintf(stderr, "Unknown token: %c\n", *(lexer->cur_tok-2));                    
                    fprintf(stderr, "Unknown token: %c\n", *(lexer->cur_tok-1));
                    fprintf(stderr, "Unknown token: %c\n", *lexer->cur_tok);
                    fprintf(stderr, "Unknown token at line %zu, col %zu: '%c' (ASCII: %d)\n", 
                            lexer->line_number, 
                            lexer->cur_tok - lexer->line_start,
                            *lexer->cur_tok,
                            (int)*lexer->cur_tok);
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

    strncpy(identifier, str_start, str_len);
    identifier[str_len] = '\0';
    //printf("Identified identifier: '%s'\n", identifier); // Debugging line
    if (keyword_token != TOKEN_IDENTIFIER) {

        add_token(lexer, keyword_token, identifier);
    } else {
        add_token(lexer, TOKEN_IDENTIFIER, identifier);
    }
}

int handle_number_token(Lexer *lexer) {
    char *str_start = lexer->cur_tok;
    int decimal_count = 0;
    
    // Handle leading decimal point (e.g., .123)
    if (*str_start == '.') {
        if (!isdigit(*(str_start + 1))) {
            return 0; 
        }
        decimal_count = 1;
    }
    
    while (isdigit(*lexer->cur_tok) || *lexer->cur_tok == '.') {
        if (*lexer->cur_tok == '.') {
            if (++decimal_count > 1) {
                fprintf(stderr, "%zu: Invalid number format — multiple decimal points\n", 
                         lexer->line_number);
                // Advance past the faulty token to attempt recovery
                while (isalnum(*lexer->cur_tok) || *lexer->cur_tok == '.') {
                    lexer->cur_tok++;
                }
                return 1;
            }
        }
        lexer->cur_tok++;
    }

    // Now copy the token string
    int str_len = lexer->cur_tok - str_start;
    char *number_str = (char *)malloc(str_len + 1);
    
    if (number_str == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for number token\n");
        return 1;
    }
    
    strncpy(number_str, str_start, str_len);
    number_str[str_len] = '\0';

    add_token(lexer, decimal_count >= 1 ? TOKEN_DECIMAL : TOKEN_NUMBER, number_str);
    
    return 0; 
}

Token handle_keyword(const char *input_word, size_t word_length) {
    
    State current_state = S_START;
    const StateNode* current_node = get_node(S_START);
    
    for (size_t i = 0; i < word_length; i++) {
        char c = input_word[i];

        if (c < 'a' || c > 'z') {
            current_state = S_IDENTIFIER;
            current_node = get_node(S_IDENTIFIER);
            continue; 
        }

        State next_state = S_IDENTIFIER; 
        bool transition_found = false;

        for (int j = 0; j < current_node->num_transitions; j++) {
            if (current_node->transitions[j].input_char == c) {
                next_state = current_node->transitions[j].next_state;
                transition_found = true;
                break;
            }
        }

         if (!transition_found || current_state == S_IDENTIFIER) {
            current_state = S_IDENTIFIER;
        } else {
            current_state = next_state;
        }
        
        current_node = get_node(current_state);
    }

    if (current_node->output != TOKEN_NONE) {
        return current_node->output;
    }

    return TOKEN_IDENTIFIER;
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

void to_lower_case(char *str, size_t length) {
    if (str == NULL) {
        return; // Handle null input safely
    }

    // Iterate through the string until the null terminator ('\0') is reached
    for (size_t i = 0; i < length; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

const StateNode MACHINE_DEF[NUM_STATES] = {
    [S_START] = {S_START, TOKEN_NONE, {
        {'a', S_A}, {'o', S_O}, {'n', S_N}, {'s', S_S}, {'e', S_E}, 
        {'c', S_C}, {'i', S_I}, {'w', S_W}, {'l', S_L}, {'m', S_M},
        {'t', S_T}, {'f', S_F}, {'b', S_B}, {'d', S_D}, {'g', S_G},
        {'r', S_R}
    }, 16},

    [S_A] = {S_A, TOKEN_NONE, {{'n', S_AN}, {'s', S_AS}, {'l', S_AL}}, 3},
    [S_AN] = {S_AN, TOKEN_NONE, {{'d', S_AND}}, 1},
    [S_AND] = {S_AND, TOKEN_AND, {{0}}, 0},
    [S_AS] = {S_AS, TOKEN_NONE, {{'k', S_ASK}}, 1},
    [S_ASK] = {S_ASK, TOKEN_ASK, {{0}}, 0},
    [S_AL] = {S_AL, TOKEN_NONE, {{'s', S_ALS}}, 1},
    [S_ALS] = {S_ALS, TOKEN_NONE, {{'o', S_ALSO}}, 1},
    [S_ALSO] = {S_ALSO, TOKEN_ALSO, {{0}}, 0},

    [S_B] = {S_B, TOKEN_NONE, {{'y', S_BY}}, 1},
    [S_BY] = {S_BY, TOKEN_BY, {{0}}, 0},

    [S_C] = {S_C, TOKEN_NONE, {{'o', S_CO}}, 1},
    [S_CO] = {S_CO, TOKEN_NONE, {{'n', S_CON}}, 1},
    [S_CON] = {S_CON, TOKEN_NONE, {{'t', S_CONT}}, 1},
    [S_CONT] = {S_CONT, TOKEN_NONE, {{'i', S_CONTI}}, 1},
    [S_CONTI] = {S_CONTI, TOKEN_NONE, {{'n', S_CONTIN}}, 1},
    [S_CONTIN] = {S_CONTIN, TOKEN_NONE, {{'u', S_CONTINU}}, 1},
    [S_CONTINU] = {S_CONTINU, TOKEN_NONE, {{'e', S_CONTINUE}}, 1},
    [S_CONTINUE] = {S_CONTINUE, TOKEN_CONTINUE, {{0}}, 0},

    [S_D] = {S_D, TOKEN_NONE, {{'e', S_DE}}, 1},
    [S_DE] = {S_DE, TOKEN_NONE, {{'c', S_DEC}}, 1},
    [S_DEC] = {S_DEC, TOKEN_NONE, {{'i', S_DECI}}, 1},
    [S_DECI] = {S_DECI, TOKEN_NONE, {{'m', S_DECIM}}, 1},
    [S_DECIM] = {S_DECIM, TOKEN_NONE, {{'a', S_DECIMA}}, 1},
    [S_DECIMA] = {S_DECIMA, TOKEN_NONE, {{'l', S_DECIMAL}}, 1},
    [S_DECIMAL] = {S_DECIMAL, TOKEN_DECIMAL, {{0}}, 0},

    [S_E] = {S_E, TOKEN_NONE, {{'n', S_EN}, {'l', S_EL}, {'x', S_EX}, {'a', S_EA}}, 4},
    [S_EN] = {S_EN, TOKEN_NONE, {{'d', S_END}}, 1},
    [S_END] = {S_END, TOKEN_END, {{0}}, 0},
    [S_EL] = {S_EL, TOKEN_NONE, {{'s', S_ELS}}, 1},
    [S_ELS] = {S_ELS, TOKEN_NONE, {{'e', S_ELSE}}, 1},
    [S_ELSE] = {S_ELSE, TOKEN_ELSE, {{0}}, 0},
    [S_EX] = {S_EX, TOKEN_NONE, {{'i', S_EXI}}, 1},
    [S_EXI] = {S_EXI, TOKEN_NONE, {{'t', S_EXIT}}, 1},
    [S_EXIT] = {S_EXIT, TOKEN_EXIT, {{0}}, 0},
    [S_EA] = {S_EA, TOKEN_NONE, {{'c', S_EAC}}, 1},
    [S_EAC] = {S_EAC, TOKEN_NONE, {{'h', S_EACH}}, 1},
    [S_EACH] = {S_EACH, TOKEN_EACH, {{0}}, 0},

    [S_F] = {S_F, TOKEN_NONE, {{'a', S_FA}}, 1},
    [S_FA] = {S_FA, TOKEN_NONE, {{'l', S_FAL}}, 1},
    [S_FAL] = {S_FAL, TOKEN_NONE, {{'s', S_FALS}}, 1},
    [S_FALS] = {S_FALS, TOKEN_NONE, {{'e', S_FALSE}}, 1},
    [S_FALSE] = {S_FALSE, TOKEN_FALSE, {{0}}, 0},

    [S_G] = {S_G, TOKEN_NONE, {{'r', S_GR}}, 1},
    [S_GR] = {S_GR, TOKEN_NONE, {{'e', S_GRE}}, 1},
    [S_GRE] = {S_GRE, TOKEN_NONE, {{'a', S_GREA}}, 1},
    [S_GREA] = {S_GREA, TOKEN_NONE, {{'t', S_GREAT}}, 1},
    [S_GREAT] = {S_GREAT, TOKEN_NONE, {{'e', S_GREATE}}, 1},
    [S_GREATE] = {S_GREATE, TOKEN_NONE, {{'r', S_GREATER}}, 1},
    [S_GREATER] = {S_GREATER, TOKEN_GREATER, {{0}}, 0},

    [S_I] = {S_I, TOKEN_NONE, {{'f', S_IF}}, 1},
    [S_IF] = {S_IF, TOKEN_IF, {{0}}, 0},

    [S_L] = {S_L, TOKEN_NONE, {{'o', S_LO}, {'e', S_LE}}, 2},
    [S_LO] = {S_LO, TOKEN_NONE, {{'o', S_LOO}}, 1},
    [S_LOO] = {S_LOO, TOKEN_NONE, {{'p', S_LOOP}}, 1},
    [S_LOOP] = {S_LOOP, TOKEN_LOOP, {{0}}, 0},
    [S_LE] = {S_LE, TOKEN_NONE, {{'t', S_LET}, {'s', S_LES}}, 2},
    [S_LET] = {S_LET, TOKEN_NONE, {{'t', S_LETT}}, 1},
    [S_LETT] = {S_LETT, TOKEN_NONE, {{'e', S_LETTE}}, 1},
    [S_LETTE] = {S_LETTE, TOKEN_NONE, {{'r', S_LETTER}}, 1},
    [S_LETTER] = {S_LETTER, TOKEN_LETTER, {{0}}, 0},
    [S_LES] = {S_LES, TOKEN_NONE, {{'s', S_LESS}}, 1},
    [S_LESS] = {S_LESS, TOKEN_LESS, {{0}}, 0},

    [S_M] = {S_M, TOKEN_NONE, {{'a', S_MA}}, 1},
    [S_MA] = {S_MA, TOKEN_NONE, {{'i', S_MAI}}, 1},
    [S_MAI] = {S_MAI, TOKEN_NONE, {{'n', S_MAIN}}, 1},
    [S_MAIN] = {S_MAIN, TOKEN_MAIN, {{0}}, 0},

    [S_N] = {S_N, TOKEN_NONE, {{'o', S_NO}, {'u', S_NU}}, 2},
    [S_NO] = {S_NO, TOKEN_NONE, {{'t', S_NOT}}, 1},
    [S_NOT] = {S_NOT, TOKEN_NOT, {{0}}, 0},
    [S_NU] = {S_NU, TOKEN_NONE, {{'m', S_NUM}}, 1},
    [S_NUM] = {S_NUM, TOKEN_NONE, {{'b', S_NUMB}}, 1},
    [S_NUMB] = {S_NUMB, TOKEN_NONE, {{'e', S_NUMBE}}, 1},
    [S_NUMBE] = {S_NUMBE, TOKEN_NONE, {{'r', S_NUMBER}}, 1},
    [S_NUMBER] = {S_NUMBER, TOKEN_NUMBER, {{0}}, 0},

    [S_O] = {S_O, TOKEN_NONE, {{'r', S_OR}, {'f', S_OF}}, 2},
    [S_OR] = {S_OR, TOKEN_OR, {{0}}, 0},
    [S_OF] = {S_OF, TOKEN_OF, {{0}}, 0},

    [S_R] = {S_R, TOKEN_NONE, {{'e', S_RE}}, 1},
    [S_RE] = {S_RE, TOKEN_NONE, {{'p', S_REP}}, 1},
    [S_REP] = {S_REP, TOKEN_NONE, {{'e', S_REPE}}, 1},
    [S_REPE] = {S_REPE, TOKEN_NONE, {{'a', S_REPEA}}, 1},
    [S_REPEA] = {S_REPEA, TOKEN_NONE, {{'t', S_REPEAT}}, 1},
    [S_REPEAT] = {S_REPEAT, TOKEN_REPEAT, {{0}}, 0},

    [S_S] = {S_S, TOKEN_NONE, {{'t', S_ST}, {'h', S_SH}}, 2},
    [S_ST] = {S_ST, TOKEN_NONE, {{'a', S_STA}, {'o', S_STO}}, 2},
    [S_STA] = {S_STA, TOKEN_NONE, {{'r', S_STAR}}, 1},
    [S_STAR] = {S_STAR, TOKEN_NONE, {{'t', S_START_KW}}, 1},
    [S_START_KW] = {S_START_KW, TOKEN_START, {{0}}, 0},
    [S_STO] = {S_STO, TOKEN_NONE, {{'p', S_STOP}}, 1},
    [S_STOP] = {S_STOP, TOKEN_STOP, {{0}}, 0},
    [S_SH] = {S_SH, TOKEN_NONE, {{'o', S_SHO}}, 1},
    [S_SHO] = {S_SHO, TOKEN_NONE, {{'w', S_SHOW}}, 1},
    [S_SHOW] = {S_SHOW, TOKEN_SHOW, {{0}}, 0},

    [S_T] = {S_T, TOKEN_NONE, {{'r', S_TR}, {'o', S_TO}, {'h', S_TH}}, 3},
    [S_TR] = {S_TR, TOKEN_NONE, {{'u', S_TRU}}, 1},
    [S_TRU] = {S_TRU, TOKEN_NONE, {{'e', S_TRUE}}, 1},
    [S_TRUE] = {S_TRUE, TOKEN_TRUE, {{0}}, 0},
    [S_TO] = {S_TO, TOKEN_TO, {{0}}, 0},
    [S_TH] = {S_TH, TOKEN_NONE, {{'a', S_THA}, {'e', S_THE}}, 2},
    [S_THA] = {S_THA, TOKEN_NONE, {{'n', S_THAN}}, 1},
    [S_THAN] = {S_THAN, TOKEN_THAN, {{0}}, 0},
    [S_THE] = {S_THE, TOKEN_NONE, {{'n', S_THEN}}, 1},
    [S_THEN] = {S_THEN, TOKEN_THEN, {{0}}, 0},

    [S_W] = {S_W, TOKEN_NONE, {{'h', S_WH}, {'o', S_WO}}, 2},
    [S_WH] = {S_WH, TOKEN_NONE, {{'i', S_WHI}}, 1},
    [S_WHI] = {S_WHI, TOKEN_NONE, {{'l', S_WHIL}}, 1},
    [S_WHIL] = {S_WHIL, TOKEN_NONE, {{'e', S_WHILE}}, 1},
    [S_WHILE] = {S_WHILE, TOKEN_WHILE, {{0}}, 0},
    [S_WO] = {S_WO, TOKEN_NONE, {{'r', S_WOR}}, 1},
    [S_WOR] = {S_WOR, TOKEN_NONE, {{'d', S_WORD}}, 1},
    [S_WORD] = {S_WORD, TOKEN_WORD, {{0}}, 0},

    [S_IDENTIFIER] = {S_IDENTIFIER, TOKEN_IDENTIFIER, {{0}}, 0}
};

const StateNode* get_node(State s) {
    if (s >= 0 && s < NUM_STATES) {
        return &MACHINE_DEF[s];
    }
    return NULL; 
}


const char *token_type_to_string(Token type) {
    switch (type) {
        case TOKEN_EOF:             return "TOKEN_EOF";
        case TOKEN_IDENTIFIER:      return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:          return "TOKEN_NUMBER";
        case TOKEN_DECIMAL:         return "TOKEN_DECIMAL";
        case TOKEN_LETTER:          return "TOKEN_LETTER";
        case TOKEN_WORD:            return "TOKEN_WORD";
        case TOKEN_TRUE:            return "TOKEN_TRUE";
        case TOKEN_FALSE:           return "TOKEN_FALSE";
        case TOKEN_SEMICOLON:       return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:           return "TOKEN_COMMA";
     
        case TOKEN_STRING_LITERAL:  return "TOKEN_STRING_LITERAL";
//      case TOKEN_ASSIGN:          return "TOKEN_ASSIGN";
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
        case TOKEN_BY:              return "TOKEN_BY";
        case TOKEN_REPEAT:          return "TOKEN_REPEAT";
        case TOKEN_SHOW:            return "TOKEN_SHOW";
        case TOKEN_TO:              return "TOKEN_TO";
        
        case TOKEN_THAN:            return "TOKEN_THAN";
        case TOKEN_EACH:            return "TOKEN_EACH";
        case TOKEN_OF:              return "TOKEN_OF";
        case TOKEN_THEN:            return "TOKEN_THEN";
        case TOKEN_ALSO:            return "TOKEN_ALSO";

        case TOKEN_SET:             return "TOKEN_SET";
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
        
        case TOKEN_EXIT:            return "TOKEN_EXIT";
        case TOKEN_LOOP:            return "TOKEN_LOOP";
        case TOKEN_MAIN:            return "TOKEN_MAIN";
        case TOKEN_NONE:            return "TOKEN_NONE";

        default:
            return "TOKEN_UNKNOWN";
    }
}