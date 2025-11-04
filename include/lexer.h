#ifndef LEXER_H  
#define LEXER_H

typedef enum {
    TOKEN_EOF,          // End-of-File
    TOKEN_IDENTIFIER,  
    TOKEN_NUMBER,
    TOKEN_DECIMAL,
    TOKEN_LETTER,
    TOKEN_WORD,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_SEMICOLON,      
    TOKEN_COMMA,          
    
    TOKEN_LPAREN, TOKEN_RPAREN,         
    TOKEN_LBRACE, TOKEN_RBRACE,         
    TOKEN_LBRACKET, TOKEN_RBRACKET,       

    //Keywords
/*  TOKEN_KEYWORD,
    TOKEN_AND,
    TOKEN_ASK,
    TOKEN_BY,
    TOKEN_NOT,
    TOKEN_OR,
    TOKEN_REPEAT,
    TOKEN_SHOW,
    TOKEN_TO,
*/
    TOKEN_START,
    TOKEN_END,
    TOKEN_CONTINUE,
    TOKEN_STOP,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,

    // Value Setting Operators
    TOKEN_SET, TOKEN_INCREASE, TOKEN_DECREASE, TOKEN_SCALE, TOKEN_SPLIT, TOKEN_REMAINDER,
    
    // Arithmetic Operators
    TOKEN_PLUS, TOKEN_MIN, TOKEN_MUL, TOKEN_DIV, TOKEN_MOD, TOKEN_IDIV, TOKEN_POW,
    
    // Single Value Operators (Unary)
    TOKEN_POS, TOKEN_NEG, TOKEN_NEXT, TOKEN_PREV,
    
    // Comparison Operators (Relational)
    TOKEN_IS, TOKEN_ISNT, TOKEN_GREATER, TOKEN_LESS,
    TOKEN_GREATEREQUAL, TOKEN_LESSEQUAL,
    
    // Logic Operators
    TOKEN_AND, TOKEN_OR, TOKEN_NOT,
    
    //Reserved Words
    TOKEN_EXIT,
    TOKEN_LOOP,
    TOKEN_MAIN
} Token;

typedef struct {
    size_t line;
    size_t col;
} Location;

typedef struct {
    Token type; // The type of the token
    char *val; // The value of the token
    Location loc; // The location of the token
} TokenData;

typedef struct {
    char *start_tok;
    char *cur_tok;
    TokenData *tokens;
    size_t token_count;
    size_t capacity;
    size_t line_number;
    char *line_start;
} Lexer;

typedef struct {
    const char* text;
    Token type;
} Keyword;


int lex(Lexer *lexer);

static void handle_identifier(Lexer *lexer);

Token handle_keyword(const char *start, size_t length);

const char* get_keyword_static_text(Token type);

void printLexerTokens(const Lexer *lexer);

void free_lexer(Lexer *lexer);

#endif