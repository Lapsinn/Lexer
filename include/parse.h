#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "lexer.h"

// ============================================================================
// Parser State
// ============================================================================

typedef struct {
    TokenData* tokens;
    size_t current;
    size_t count;
    int has_error;
} Parser;

// ============================================================================
// Lifecycle
// ============================================================================

Parser* parser_create(TokenData* tokens, size_t count);
void parser_destroy(Parser* parser);

// ============================================================================
// Main Entry Point
// ============================================================================

ASTNode* parse_program(Parser* parser);

// ============================================================================
// Grammar Functions (Exposed for testing/internal use)
// ============================================================================

ASTNode* parse_stmt_list(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_block(Parser* parser);

// Declarations
ASTNode* parse_global_declaration(Parser* parser);
ASTNode* parse_func_decl(Parser* parser);
ASTNode* parse_var_decl_wrapper(Parser* parser, int is_const);
ASTNode* parse_decl_stmt(Parser* parser);
ASTNode* parse_param_list(Parser* parser);

// Statements
ASTNode* parse_simple_stmt(Parser* parser);
ASTNode* parse_compound_stmt(Parser* parser);
ASTNode* parse_ass_stmt(Parser* parser);
ASTNode* parse_input_stmt(Parser* parser);
ASTNode* parse_output_stmt(Parser* parser);
ASTNode* parse_cond_stmt(Parser* parser);
ASTNode* parse_iter_stmt(Parser* parser);
ASTNode* parse_return_stmt(Parser* parser);

// Expressions
ASTNode* parse_expr(Parser* parser);
ASTNode* parse_value(Parser* parser);
ASTNode* parse_arg_list(Parser* parser);
ASTNode* parse_fn_call(Parser* parser);
ASTNode* parse_input_expr(Parser* parser);
ASTNode* parse_sizeof(Parser* parser);
ASTNode* parse_unary(Parser* parser);
ASTNode* parse_factor(Parser* parser);
ASTNode* parse_term(Parser* parser);
ASTNode* parse_arith_expr(Parser* parser);
ASTNode* parse_relational(Parser* parser);
ASTNode* parse_equality(Parser* parser);
ASTNode* parse_bool_and(Parser* parser);
ASTNode* parse_bool_or(Parser* parser);

// ============================================================================
// Utilities & Helpers
// ============================================================================

void consume_token(Parser* parser);
TokenData peek(Parser* parser);
TokenData peek_ahead(Parser* parser, int offset);
int check(Parser* parser, Token type);
int expect(Parser* parser, Token type, const char* message);
void error(Parser* parser, const char* message);
int is_at_end(Parser* parser);

void synchronize(Parser* parser);
int is_statement_start(Token token);

int is_data_type(Token token);
int is_assignment_operator(Token token);
int is_relational_operator(Token token);

int map_assign_operator(Token token);
int map_binary_operator(Token token);
int map_unary_operator(Token token);

#endif // PARSER_H