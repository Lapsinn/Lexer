#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "lexer.h" // Needed for TokenData in create_node_with_loc

// AST Node Types
typedef enum {
    // Program structure
    NODE_PROGRAM,
    NODE_STATEMENT_LIST,
    NODE_PARAM_LIST,
    NODE_ARG_LIST,
    
    // Statements
    NODE_DECL_STMT,
    NODE_VAR_DECL,
    NODE_FUNC_DECL,
    NODE_ASSIGN_STMT,
    NODE_INPUT_STMT,
    NODE_OUTPUT_STMT,
    NODE_COND_STMT,
    NODE_ITER_STMT,
    NODE_RETURN_STMT,
    NODE_CONTINUE_STMT,
    NODE_STOP_STMT,
    NODE_BLOCK,
    NODE_FUNC_CALL,
    
    // Expressions
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_LITERAL,
    NODE_IDENTIFIER
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// ============================================================================
// Identifier and Literal Nodes
// ============================================================================

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

// ============================================================================
// Expression Nodes
// ============================================================================

typedef struct {
    enum {
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_IDIV,
        OP_POW,
        OP_EQ,
        OP_NEQ,
        OP_LT,
        OP_GT,
        OP_LTE,
        OP_GTE,
        OP_AND,
        OP_OR
    } operator;
    ASTNode* left;
    ASTNode* right;
} BinaryExpressionNode;

typedef struct {
    enum {
        UNOP_NEG,
        UNOP_NOT,
        UNOP_POS
    } operator;
    ASTNode* operand;
} UnaryExpressionNode;

// ============================================================================
// Declaration Nodes
// ============================================================================

typedef struct {
    char* param_type;
    char* param_name;
} ParameterNode;

typedef struct {
    ParameterNode** parameters;
    size_t count;
    size_t capacity;
} ParameterListNode;

typedef struct {
    char* var_name;
    char* data_type;
    ASTNode* init_expr;
    int is_const;
} VarDeclNode;

typedef struct {
    char* return_type;
    char* func_name;
    ASTNode* params;
    ASTNode* body;
} FuncDeclNode;

// ============================================================================
// Statement Nodes
// ============================================================================

typedef struct {
    ASTNode* decl;
} DeclStmtNode;

typedef struct {
    char* var_name;
    enum {
        ASSIGN_DIRECT,
        ASSIGN_PLUS,
        ASSIGN_MINUS,
        ASSIGN_MUL,
        ASSIGN_DIV,
        ASSIGN_MOD
    } assign_type;
    ASTNode* expr;
} AssignStmtNode;

typedef struct {
    char* var_name;
    char* data_type;
} InputStmtNode;

typedef struct {
    ASTNode* expr;
} OutputStmtNode;

typedef struct {
    ASTNode* condition;
    ASTNode* then_body;
    ASTNode* else_body;
} CondStmtNode;

typedef struct {
    ASTNode* init;
    ASTNode* condition;
    ASTNode* increment;
    ASTNode* body;
} IterStmtNode;

typedef struct {
    ASTNode* expr;
} ReturnStmtNode;

// ============================================================================
// Function Call Node
// ============================================================================

typedef struct {
    char* func_name;
    ASTNode** arguments;
    size_t count;
    size_t capacity;
} FuncCallNode;

// ============================================================================
// Block and List Nodes
// ============================================================================

typedef struct {
    ASTNode** statements;
    size_t count;
    size_t capacity;
} StatementListNode;

typedef struct {
    ASTNode** args;
    size_t count;
    size_t capacity;
} ArgListNode;

// ============================================================================
// Program Node
// ============================================================================

typedef struct {
    ASTNode** global_decls;
    size_t global_count;
    size_t global_capacity;
    ASTNode* stmt_list;
} ProgramNode;

// ============================================================================
// Generic AST Node
// ============================================================================

struct ASTNode {
    ASTNodeType type;
    ASTNode* parent;
    void* specific_node;
    
    size_t line;
    size_t column;
};

// ============================================================================
// AST Management Functions
// ============================================================================

// Core creation
ASTNode* create_node(ASTNodeType type, void* specific_data);

// IMPROVEMENT: Helper to create node and set location from token in one step
ASTNode* create_node_with_loc(ASTNodeType type, void* specific_data, TokenData loc_token);

// Memory cleanup
void free_ast(ASTNode* root);

// List management (Dynamic Arrays)
void add_statement(StatementListNode* list, ASTNode* stmt);
void add_global_decl(ProgramNode* program, ASTNode* decl);
void add_parameter(ParameterListNode* list, ParameterNode* param);
void add_argument(FuncCallNode* call, ASTNode* arg);

// Debugging
void print_ast(ASTNode* node, int indent);

#endif // AST_H