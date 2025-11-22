#ifndef LEXER_H  
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_NONE = 0,
    TOKEN_IDENTIFIER,  
    TOKEN_NUMBER,
    TOKEN_DECIMAL,
    TOKEN_LETTER,
    TOKEN_CHAR_LITERAL,
    TOKEN_WORD,
    TOKEN_STRING_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_SEMICOLON,      
    TOKEN_COMMA,          
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_ASSIGN,
    TOKEN_CONST,
    TOKEN_NULL,
    TOKEN_SIZEOF,
    TOKEN_RETURN,

    TOKEN_LPAREN, TOKEN_RPAREN,         
    TOKEN_LBRACE, TOKEN_RBRACE,         
    TOKEN_LBRACKET, TOKEN_RBRACKET,       

    //Keywords
    TOKEN_AND,
    TOKEN_ASK,
    TOKEN_BY,
    TOKEN_NOT,
    TOKEN_OR,
    TOKEN_REPEAT,
    TOKEN_SHOW,
    TOKEN_TO,

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

    // Noise words
    TOKEN_THAN,
    TOKEN_EACH,
    TOKEN_OF,
    TOKEN_THEN,
    TOKEN_ALSO,
    
    //Reserved Words
    TOKEN_IMPORT,
    TOKEN_GOTO,
    TOKEN_EXIT,
    TOKEN_LOOP,
    TOKEN_MAIN,
    TOKEN_EOF
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

//  State Definitions (FSM Internal IDs)
typedef enum {
    S_START = 0, 

    S_A, S_AN, S_AND, 
    S_AS, S_ASK, 
    S_AL, S_ALS, S_ALSO,

    S_O, S_OR, S_OF,

    S_N, S_NO, S_NOT,
    S_NU, S_NUM, S_NUMB, S_NUMBE, S_NUMBER,
    S_NUL, S_NULL,

    S_S, S_ST, S_STA, S_STAR, S_START_KW, S_STO, S_STOP,
    S_SH, S_SHO, S_SHOW,
    S_SI, S_SIZ, S_SIZE, S_SIZEO, S_SIZEOF, 

    S_E, S_EN, S_END, S_EL, S_ELS, S_ELSE, S_EX, S_EXI, S_EXIT,
    S_EA, S_EAC, S_EACH,

    S_C, S_CO, S_CON, 
    S_CONS, S_CONST, 
    S_CONT, S_CONTI, S_CONTIN, S_CONTINU, S_CONTINUE,

    S_I, S_IF, 
    S_IM, S_IMP, S_IMPO, S_IMPOR, S_IMPORT, 
    
    S_W, S_WH, S_WHI, S_WHIL, S_WHILE,
    S_WO, S_WOR, S_WORD,

    S_L, S_LO, S_LOO, S_LOOP,
    S_LE, S_LET, S_LETT, S_LETTE, S_LETTER,
    S_LES, S_LESS,

    S_M, S_MA, S_MAI, S_MAIN,

    S_T, S_TR, S_TRU, S_TRUE,
    S_TH, S_THE, S_THEN,

    S_F, S_FA, S_FAL, S_FALS, S_FALSE,

    S_D, S_DE, S_DEC, S_DECI, S_DECIM, S_DECIMA, S_DECIMAL,

    S_G, S_GR, S_GRE, S_GREA, S_GREAT, S_GREATE, S_GREATER,
    S_GO, S_GOT, S_GOTO, 

    S_R, S_RE, S_REP, S_REPE, S_REPEA, S_REPEAT,
    S_RET, S_RETU, S_RETUR, S_RETURN, 

    S_IDENT, 
    NUM_STATES
} State;

#define MAX_TRANSITIONS 26 


// FSM Data Structures
typedef struct {
    char input_char;
    State next_state;
} Transition;

typedef struct {
    State id;
    Token output; 
    Transition transitions[MAX_TRANSITIONS];
    int num_transitions;
} StateNode;

const StateNode* get_node(State s);

int lex(Lexer *lexer);

static void add_token(Lexer *lexer, Token type, char *val);

static void handle_identifier(Lexer *lexer);

const char *token_type_to_string(Token type);

Token handle_keyword(const char *start, size_t length);

const char* get_keyword_static_text(Token type);

void printLexerTokens(const Lexer *lexer);

void free_lexer(Lexer *lexer);

int handle_number_token(Lexer *lexer);

//void to_lower_case(char *str, size_t length)

#endif