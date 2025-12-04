#ifndef AST_H
#define AST_H

#include <stddef.h>

// AST Node Types
typedef enum {
    // Program structure
    NODE_PROGRAM,
    NODE_STATEMENT_LIST,
    
    // Statements
    NODE_DECL_STMT,
    NODE_VAR_DECL,
    NODE_ASSIGN_STMT,
    NODE_INPUT_STMT,
    NODE_OUTPUT_STMT,
    NODE_COND_STMT,
    NODE_ITER_STMT,
    NODE_RETURN_STMT,
    NODE_CONTINUE_STMT,
    NODE_STOP_STMT,
    NODE_BLOCK,
    
    // Expressions
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_FUNCTION_CALL,
    
    // Logical/Relational
    NODE_LOGICAL_EXPR,
    NODE_RELATION_EXPR,
    
    // Special
    NODE_IF_ELSE,
    NODE_PARAM_LIST,
    NODE_ARG_LIST
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// Specific node structures
typedef struct {
    char* name;
} IdentifierNode;

typedef struct {
    enum {
        LITERAL_NUMBER,
        LITERAL_DECIMAL,
        LITERAL_STRING,
        LITERAL_CHAR,
        LITERAL_BOOL,
        LITERAL_NULL
    } literal_type;
    union {
        int int_value;
        double double_value;
        char* string_value;
        char char_value;
        int bool_value;
    } value;
} LiteralNode;

typedef struct {
    enum {
        OP_ADD,      // +
        OP_SUB,      // -
        OP_MUL,      // *
        OP_DIV,      // /
        OP_MOD,      // %
        OP_IDIV,     // ~
        OP_POW,      // ^
        OP_EQ,       // ==
        OP_NEQ,      // !=
        OP_LT,       // <
        OP_GT,       // >
        OP_LTE,      // <=
        OP_GTE,      // >=
        OP_AND,      // &&
        OP_OR        // ||
    } operator;
    ASTNode* left;
    ASTNode* right;
} BinaryExpressionNode;

typedef struct {
    enum {
        UNOP_NEG,    // - (negation)
        UNOP_NOT,    // ! (logical not)
        UNOP_POS     // + (positive)
    } operator;
    ASTNode* operand;
} UnaryExpressionNode;

typedef struct {
    ASTNode** statements;
    size_t count;
    size_t capacity;
} StatementListNode;

typedef struct {
    char* var_name;
    char* data_type;  // "number", "decimal", "letter", etc.
    ASTNode* init_expr;  // NULL if no initialization
    int is_const;
} VarDeclNode;

typedef struct {
    char* var_name;
    enum {
        ASSIGN_DIRECT,      // =
        ASSIGN_PLUS,        // +=
        ASSIGN_MINUS,       // -=
        ASSIGN_MUL,         // *=
        ASSIGN_DIV,         // /=
        ASSIGN_MOD          // %=
    } assign_type;
    ASTNode* expr;
} AssignStmtNode;

typedef struct {
    ASTNode* expr;  // Expression to output
} OutputStmtNode;

typedef struct {
    char* var_name;
    char* data_type;  // Type to read
} InputStmtNode;

typedef struct {
    ASTNode* condition;
    ASTNode* then_body;
    ASTNode* else_body;  // NULL if no else
} CondStmtNode;

typedef struct {
    ASTNode* init;       // Initialization
    ASTNode* condition;  // Loop condition
    ASTNode* increment;  // Increment statement
    ASTNode* body;       // Loop body
} IterStmtNode;

typedef struct {
    ASTNode* expr;  // NULL for void return
} ReturnStmtNode;

// Generic AST Node
struct ASTNode {
    ASTNodeType type;
    ASTNode* parent;
    void* specific_node;
    
    // Source location info (optional, for error reporting)
    int line;
    int column;
};

// AST Management Functions
ASTNode* create_node(ASTNodeType type, void* specific_data);
void free_ast(ASTNode* root);

// Helper function to add statement to list
void add_statement(StatementListNode* list, ASTNode* stmt);

#endif // AST_H
