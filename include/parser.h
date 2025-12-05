#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

// Parser state
typedef struct {
    TokenData* tokens;
    size_t current;
    size_t count;
    int has_error;
} Parser;

// Main entry point
ASTNode* parse_program(Parser* parser);

// Statement parsing
ASTNode* parse_statement_list(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_decl_stmt(Parser* parser);
ASTNode* parse_assign_stmt(Parser* parser);
ASTNode* parse_output_stmt(Parser* parser);
ASTNode* parse_input_stmt(Parser* parser);
ASTNode* parse_cond_stmt(Parser* parser);
ASTNode* parse_iter_stmt(Parser* parser);
ASTNode* parse_return_stmt(Parser* parser);
ASTNode* parse_block(Parser* parser);

// Expression parsing (following precedence hierarchy)
ASTNode* parse_expr(Parser* parser);           // Handles +, -
ASTNode* parse_logical_or(Parser* parser);     // Handles ||
ASTNode* parse_logical_and(Parser* parser);    // Handles &&
ASTNode* parse_comparison(Parser* parser);     // Handles ==, !=, <, >, <=, >=
ASTNode* parse_term(Parser* parser);           // Handles *, /, %, ~
ASTNode* parse_factor(Parser* parser);         // Handles ^
ASTNode* parse_unary(Parser* parser);          // Handles unary -, +, !
ASTNode* parse_primary(Parser* parser);        // Handles literals, identifiers, ()

// Parser utilities
void consume_token(Parser* parser);
Token peek(Parser* parser);
Token peek_ahead(Parser* parser, int offset);
int check(Parser* parser, Token type);
int expect(Parser* parser, Token type, const char* message);
void error(Parser* parser, const char* message);
int is_at_end(Parser* parser);

// Helper functions
int is_data_type(Token token);
int is_assignment_operator(Token token);

#endif // PARSER_H