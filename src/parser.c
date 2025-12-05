// ULTRA-ROBUST PARSER - GUARANTEED TREE GENERATION
// Key: Recover from ANY error, no matter how severe

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_statement_list(Parser* parser);
ASTNode* parse_block(Parser* parser);
ASTNode* parse_logical_or(Parser* parser);
ASTNode* parse_logical_and(Parser* parser);
ASTNode* parse_comparison(Parser* parser);
ASTNode* parse_expr(Parser* parser);
ASTNode* parse_term(Parser* parser);
ASTNode* parse_factor(Parser* parser);
ASTNode* parse_unary(Parser* parser);
ASTNode* parse_primary(Parser* parser);

// ============================================================================
// UTILITIES
// ============================================================================

void consume_token(Parser* parser) {
    if (!is_at_end(parser)) parser->current++;
}

Token peek(Parser* parser) {
    return (parser->current < parser->count) ? parser->tokens[parser->current].type : TOKEN_EOF;
}

Token peek_ahead(Parser* parser, int offset) {
    size_t pos = parser->current + offset;
    return (pos < parser->count) ? parser->tokens[pos].type : TOKEN_EOF;
}

int check(Parser* parser, Token type) {
    return peek(parser) == type;
}

int expect(Parser* parser, Token type, const char* message) {
    if (check(parser, type)) {
        consume_token(parser);
        return 1;
    }
    error(parser, message);
    return 0;
}

void error(Parser* parser, const char* message) {
    if (parser->current < parser->count) {
        TokenData* tok = &parser->tokens[parser->current];
        fprintf(stderr, "Parse Error at line %zu, col %zu: %s (got '%s')\n",
                tok->loc.line, tok->loc.col, message, tok->val ? tok->val : "");
    } else {
        fprintf(stderr, "Parse Error: %s (at EOF)\n", message);
    }
    parser->has_error = 1;
}

int is_at_end(Parser* parser) {
    return peek(parser) == TOKEN_EOF;
}

int is_data_type(Token token) {
    return token == TOKEN_NUMBER || token == TOKEN_DECIMAL ||
           token == TOKEN_LETTER || token == TOKEN_WORD || token == TOKEN_BOOL;
}

int is_assignment_operator(Token token) {
    return token == TOKEN_ASSIGN || token == TOKEN_PLUS_EQUAL ||
           token == TOKEN_MINUS_EQUAL || token == TOKEN_MUL_EQUAL ||
           token == TOKEN_DIV_EQUAL || token == TOKEN_MOD_EQUAL;
}

// ============================================================================
// AGGRESSIVE ERROR RECOVERY
// ============================================================================

/**
 * ULTRA-AGGRESSIVE: Skip to ANY of these tokens
 * Guaranteed to recover no matter what
 */
void synchronize_aggressive(Parser* parser) {
    while (!is_at_end(parser)) {
        // Safe termination points
        if (peek(parser) == TOKEN_SEMICOLON) {
            consume_token(parser);
            return;
        }
        if (peek(parser) == TOKEN_END || peek(parser) == TOKEN_RBRACE) {
            return;
        }
        
        // Safe statement starters
        Token tok = peek(parser);
        if (is_data_type(tok) || tok == TOKEN_CONST || tok == TOKEN_IF || 
            tok == TOKEN_REPEAT || tok == TOKEN_SHOW || tok == TOKEN_RETURN ||
            tok == TOKEN_IDENTIFIER || tok == TOKEN_CONTINUE || tok == TOKEN_STOP) {
            return;
        }
        
        // Skip junk
        consume_token(parser);
    }
}

/**
 * Helper: Create dummy nodes that never fail
 */
ASTNode* create_dummy_literal(void) {
    LiteralNode* lit = malloc(sizeof(LiteralNode));
    lit->literal_type = LITERAL_NUMBER;
    lit->value.int_value = 0;
    return create_node(NODE_LITERAL, lit);
}

ASTNode* create_dummy_identifier(void) {
    IdentifierNode* id = malloc(sizeof(IdentifierNode));
    id->name = strdup("_error_");
    return create_node(NODE_IDENTIFIER, id);
}

ASTNode* create_dummy_statement(void) {
    return create_dummy_literal();
}

// ============================================================================
// STATEMENT PARSING - ALL WITH RECOVERY
// ============================================================================

ASTNode* parse_decl_stmt(Parser* parser) {
    int is_const = 0;
    if (check(parser, TOKEN_CONST)) {
        is_const = 1;
        consume_token(parser);
    }
    
    if (!is_data_type(peek(parser))) {
        error(parser, "Expected data type in declaration");
        synchronize_aggressive(parser);
        return create_dummy_statement();
    }
    
    char* data_type = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected identifier in declaration");
        free(data_type);
        synchronize_aggressive(parser);
        return create_dummy_statement();
    }
    
    char* var_name = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    ASTNode* init_expr = NULL;
    if (check(parser, TOKEN_ASSIGN)) {
        consume_token(parser);
        init_expr = parse_expr(parser);
        if (!init_expr) init_expr = create_dummy_literal();
    }
    
    VarDeclNode* decl = malloc(sizeof(VarDeclNode));
    decl->var_name = var_name;
    decl->data_type = data_type;
    decl->init_expr = init_expr;
    decl->is_const = is_const;
    
    return create_node(NODE_VAR_DECL, decl);
}

ASTNode* parse_assign_stmt(Parser* parser) {
    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected identifier in assignment");
        return create_dummy_statement();
    }
    
    char* var_name = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    if (!is_assignment_operator(peek(parser))) {
        error(parser, "Expected assignment operator");
        free(var_name);
        return create_dummy_statement();
    }
    
    Token op = peek(parser);
    consume_token(parser);
    ASTNode* expr = parse_expr(parser);
    if (!expr) expr = create_dummy_literal();
    
    AssignStmtNode* assign = malloc(sizeof(AssignStmtNode));
    assign->var_name = var_name;
    assign->expr = expr;
    
    switch (op) {
        case TOKEN_ASSIGN: assign->assign_type = ASSIGN_DIRECT; break;
        case TOKEN_PLUS_EQUAL: assign->assign_type = ASSIGN_PLUS; break;
        case TOKEN_MINUS_EQUAL: assign->assign_type = ASSIGN_MINUS; break;
        case TOKEN_MUL_EQUAL: assign->assign_type = ASSIGN_MUL; break;
        case TOKEN_DIV_EQUAL: assign->assign_type = ASSIGN_DIV; break;
        case TOKEN_MOD_EQUAL: assign->assign_type = ASSIGN_MOD; break;
        default: assign->assign_type = ASSIGN_DIRECT; break;
    }
    
    return create_node(NODE_ASSIGN_STMT, assign);
}

ASTNode* parse_output_stmt(Parser* parser) {
    if (!check(parser, TOKEN_SHOW)) {
        error(parser, "Expected 'show'");
        return create_dummy_statement();
    }
    consume_token(parser);
    
    // Recover missing (
    if (!check(parser, TOKEN_LPAREN)) {
        error(parser, "Expected '(' after 'show'");
    } else {
        consume_token(parser);
    }
    
    ASTNode* expr = NULL;
    if (!check(parser, TOKEN_RPAREN)) {
        expr = parse_expr(parser);
    }
    if (!expr) expr = create_dummy_literal();
    
    // Recover missing )
    if (!check(parser, TOKEN_RPAREN)) {
        error(parser, "Expected ')' after expression");
    } else {
        consume_token(parser);
    }
    
    OutputStmtNode* output = malloc(sizeof(OutputStmtNode));
    output->expr = expr;
    return create_node(NODE_OUTPUT_STMT, output);
}

ASTNode* parse_input_stmt(Parser* parser) {
    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected identifier before 'ask'");
        return create_dummy_statement();
    }
    
    char* var_name = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    if (!expect(parser, TOKEN_ASSIGN, "Expected '=' in input statement")) {
        free(var_name);
        return create_dummy_statement();
    }
    
    if (!expect(parser, TOKEN_ASK, "Expected 'ask'")) {
        free(var_name);
        return create_dummy_statement();
    }
    
    if (!check(parser, TOKEN_LPAREN)) {
        error(parser, "Expected '(' after 'ask'");
    } else {
        consume_token(parser);
    }
    
    char* data_type = strdup("number");
    if (is_data_type(peek(parser))) {
        free(data_type);
        data_type = strdup(parser->tokens[parser->current].val);
        consume_token(parser);
    } else {
        error(parser, "Expected data type in ask statement");
    }
    
    if (!check(parser, TOKEN_RPAREN)) {
        error(parser, "Expected ')' after data type");
    } else {
        consume_token(parser);
    }
    
    InputStmtNode* input = malloc(sizeof(InputStmtNode));
    input->var_name = var_name;
    input->data_type = data_type;
    return create_node(NODE_INPUT_STMT, input);
}

ASTNode* parse_cond_stmt(Parser* parser) {
    if (!expect(parser, TOKEN_IF, "Expected 'if'")) {
        return create_dummy_statement();
    }
    
    if (!check(parser, TOKEN_LPAREN)) {
        error(parser, "Expected '(' after 'if'");
    } else {
        consume_token(parser);
    }
    
    ASTNode* condition = parse_logical_or(parser);
    if (!condition) condition = create_dummy_literal();
    
    if (!check(parser, TOKEN_RPAREN)) {
        error(parser, "Expected ')' after condition");
    } else {
        consume_token(parser);
    }
    
    if (check(parser, TOKEN_THEN)) consume_token(parser);
    
    ASTNode* then_body = NULL;
    if (check(parser, TOKEN_LBRACE)) {
        then_body = parse_block(parser);
    } else {
        error(parser, "Expected '{' for if body");
        then_body = parse_statement(parser);
        if (!then_body) then_body = create_dummy_statement();
    }
    
    ASTNode* else_body = NULL;
    if (check(parser, TOKEN_ELSE)) {
        consume_token(parser);
        if (check(parser, TOKEN_IF)) {
            else_body = parse_cond_stmt(parser);
        } else if (check(parser, TOKEN_LBRACE)) {
            else_body = parse_block(parser);
        } else {
            error(parser, "Expected '{' or 'if' after 'else'");
            else_body = parse_statement(parser);
            if (!else_body) else_body = create_dummy_statement();
        }
    }
    
    CondStmtNode* cond = malloc(sizeof(CondStmtNode));
    cond->condition = condition;
    cond->then_body = then_body;
    cond->else_body = else_body;
    return create_node(NODE_COND_STMT, cond);
}

ASTNode* parse_iter_stmt(Parser* parser) {
    if (!expect(parser, TOKEN_REPEAT, "Expected 'repeat'")) {
        return create_dummy_statement();
    }
    
    if (!check(parser, TOKEN_LPAREN)) {
        error(parser, "Expected '(' after 'repeat'");
    } else {
        consume_token(parser);
    }
    
    ASTNode* init = parse_statement(parser);
    if (!init) init = create_dummy_statement();
    
    // Skip to semicolon aggressively
    while (!check(parser, TOKEN_SEMICOLON) && !is_at_end(parser) && 
           !check(parser, TOKEN_END) && !check(parser, TOKEN_RBRACE)) {
        consume_token(parser);
    }
    if (check(parser, TOKEN_SEMICOLON)) consume_token(parser);
    
    ASTNode* condition = parse_logical_or(parser);
    if (!condition) condition = create_dummy_literal();
    
    // Skip to semicolon aggressively
    while (!check(parser, TOKEN_SEMICOLON) && !is_at_end(parser) && 
           !check(parser, TOKEN_END) && !check(parser, TOKEN_RBRACE)) {
        consume_token(parser);
    }
    if (check(parser, TOKEN_SEMICOLON)) consume_token(parser);
    
    ASTNode* increment = parse_statement(parser);
    if (!increment) increment = create_dummy_statement();
    
    if (!check(parser, TOKEN_RPAREN)) {
        error(parser, "Expected ')' after increment");
    } else {
        consume_token(parser);
    }
    
    ASTNode* body = NULL;
    if (check(parser, TOKEN_LBRACE)) {
        body = parse_block(parser);
    } else {
        error(parser, "Expected '{' for loop body");
        body = parse_statement(parser);
        if (!body) body = create_dummy_statement();
    }
    
    IterStmtNode* iter = malloc(sizeof(IterStmtNode));
    iter->init = init;
    iter->condition = condition;
    iter->increment = increment;
    iter->body = body;
    return create_node(NODE_ITER_STMT, iter);
}

ASTNode* parse_return_stmt(Parser* parser) {
    if (!expect(parser, TOKEN_RETURN, "Expected 'return'")) {
        return create_dummy_statement();
    }
    
    ASTNode* expr = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) {
        expr = parse_expr(parser);
    }
    if (!expr) expr = create_dummy_literal();
    
    ReturnStmtNode* ret = malloc(sizeof(ReturnStmtNode));
    ret->expr = expr;
    return create_node(NODE_RETURN_STMT, ret);
}

ASTNode* parse_block(Parser* parser) {
    if (!check(parser, TOKEN_LBRACE)) {
        error(parser, "Expected '{' to start block");
    } else {
        consume_token(parser);
    }
    
    ASTNode* stmt_list = parse_statement_list(parser);
    
    if (!check(parser, TOKEN_RBRACE)) {
        error(parser, "Expected '}' to end block");
        // Try to find closing brace or end token
        while (!is_at_end(parser) && !check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_END)) {
            consume_token(parser);
        }
    } else {
        consume_token(parser);
    }
    
    return create_node(NODE_BLOCK, stmt_list);
}

// ============================================================================
// STATEMENT LIST - GUARANTEED COMPLETION
// ============================================================================

/**
 * ULTRA-AGGRESSIVE: Always reaches end token
 * Never returns NULL
 * Never gives up
 */
ASTNode* parse_statement_list(Parser* parser) {
    StatementListNode* list = malloc(sizeof(StatementListNode));
    list->statements = NULL;
    list->count = 0;
    list->capacity = 0;
    
    while (!check(parser, TOKEN_END) && !check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
        // SKIP EMPTY STATEMENTS (multiple semicolons)
        if (check(parser, TOKEN_SEMICOLON)) {
            consume_token(parser);
            continue;
        }
        
        ASTNode* stmt = parse_statement(parser);
        
        // ALWAYS add statement
        if (stmt) {
            add_statement(list, stmt);
        }
        
        // Consume semicolon if present, but don't fail if missing
        if (check(parser, TOKEN_SEMICOLON)) {
            consume_token(parser);
        } else {
            // Aggressive recovery: skip to next safe token IF not a block/control statement
            if (stmt && stmt->type != NODE_COND_STMT && 
                stmt->type != NODE_ITER_STMT && stmt->type != NODE_BLOCK) {
                
                // Only error if next token is suspicious
                Token next = peek(parser);
                if (next != TOKEN_END && next != TOKEN_RBRACE && next != TOKEN_EOF &&
                    !is_data_type(next) && next != TOKEN_IF && next != TOKEN_REPEAT && 
                    next != TOKEN_SHOW && next != TOKEN_IDENTIFIER && 
                    next != TOKEN_RETURN && next != TOKEN_CONTINUE && next != TOKEN_STOP) {
                    error(parser, "Expected ';' after statement");
                    // Still continue, don't sync
                }
            }
        }
    }
    
    return create_node(NODE_STATEMENT_LIST, list);
}

ASTNode* parse_program(Parser* parser) {
    if (!expect(parser, TOKEN_START, "Expected 'start' at beginning")) {
        // Try to recover
        synchronize_aggressive(parser);
    }
    
    ASTNode* stmt_list = parse_statement_list(parser);
    
    if (!expect(parser, TOKEN_END, "Expected 'end' at end of program")) {
        // Still return program even if no end
    }
    
    return create_node(NODE_PROGRAM, stmt_list);
}

// ============================================================================
// EXPRESSIONS - ALL GUARANTEED NON-NULL
// ============================================================================

ASTNode* parse_primary(Parser* parser) {
    Token tok = peek(parser);
    
    if (tok == TOKEN_INTEGER) {
        LiteralNode* lit = malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_NUMBER;
        lit->value.int_value = atoi(parser->tokens[parser->current].val);
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    if (tok == TOKEN_FLOAT) {
        LiteralNode* lit = malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_DECIMAL;
        lit->value.double_value = atof(parser->tokens[parser->current].val);
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    if (tok == TOKEN_STR_LIT) {
        LiteralNode* lit = malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_STRING;
        lit->value.string_value = strdup(parser->tokens[parser->current].val);
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    if (tok == TOKEN_CHAR_LIT) {
        LiteralNode* lit = malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_CHAR;
        lit->value.char_value = parser->tokens[parser->current].val[0];
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    if (tok == TOKEN_TRUE || tok == TOKEN_FALSE) {
        LiteralNode* lit = malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_BOOL;
        lit->value.bool_value = (tok == TOKEN_TRUE) ? 1 : 0;
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    if (tok == TOKEN_NULL) {
        LiteralNode* lit = malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_NULL;
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    if (tok == TOKEN_IDENTIFIER) {
        IdentifierNode* id = malloc(sizeof(IdentifierNode));
        id->name = strdup(parser->tokens[parser->current].val);
        consume_token(parser);
        return create_node(NODE_IDENTIFIER, id);
    }
    if (tok == TOKEN_LPAREN) {
        consume_token(parser);
        ASTNode* expr = parse_expr(parser);
        if (!expr) expr = create_dummy_literal();
        
        if (!check(parser, TOKEN_RPAREN)) {
            error(parser, "Expected ')' after expression");
        } else {
            consume_token(parser);
        }
        return expr;
    }
    
    error(parser, "Expected expression");
    return create_dummy_literal();
}

ASTNode* parse_unary(Parser* parser) {
    Token tok = peek(parser);
    
    if (tok == TOKEN_MIN || tok == TOKEN_PLUS || tok == TOKEN_NOT) {
        consume_token(parser);
        UnaryExpressionNode* unary = malloc(sizeof(UnaryExpressionNode));
        unary->operator = (tok == TOKEN_MIN) ? UNOP_NEG : (tok == TOKEN_PLUS) ? UNOP_POS : UNOP_NOT;
        unary->operand = parse_unary(parser);
        if (!unary->operand) unary->operand = create_dummy_literal();
        return create_node(NODE_UNARY_EXPR, unary);
    }
    
    return parse_primary(parser);
}

ASTNode* parse_factor(Parser* parser) {
    ASTNode* left = parse_unary(parser);
    if (!left) left = create_dummy_literal();
    
    if (check(parser, TOKEN_POW)) {
        consume_token(parser);
        BinaryExpressionNode* bin = malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_POW;
        bin->left = left;
        bin->right = parse_factor(parser);
        if (!bin->right) bin->right = create_dummy_literal();
        return create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* left = parse_factor(parser);
    if (!left) left = create_dummy_literal();
    
    while (check(parser, TOKEN_MUL) || check(parser, TOKEN_DIV) ||
           check(parser, TOKEN_MOD) || check(parser, TOKEN_IDIV)) {
        Token op = peek(parser);
        consume_token(parser);
        
        BinaryExpressionNode* bin = malloc(sizeof(BinaryExpressionNode));
        bin->left = left;
        bin->right = parse_factor(parser);
        if (!bin->right) bin->right = create_dummy_literal();
        
        switch (op) {
            case TOKEN_MUL: bin->operator = OP_MUL; break;
            case TOKEN_DIV: bin->operator = OP_DIV; break;
            case TOKEN_MOD: bin->operator = OP_MOD; break;
            case TOKEN_IDIV: bin->operator = OP_IDIV; break;
            default: break;
        }
        
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

ASTNode* parse_expr(Parser* parser) {
    ASTNode* left = parse_term(parser);
    if (!left) left = create_dummy_literal();
    
    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MIN)) {
        Token op = peek(parser);
        consume_token(parser);
        
        BinaryExpressionNode* bin = malloc(sizeof(BinaryExpressionNode));
        bin->operator = (op == TOKEN_PLUS) ? OP_ADD : OP_SUB;
        bin->left = left;
        bin->right = parse_term(parser);
        if (!bin->right) bin->right = create_dummy_literal();
        
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

ASTNode* parse_comparison(Parser* parser) {
    ASTNode* left = parse_expr(parser);
    if (!left) left = create_dummy_literal();
    
    if (check(parser, TOKEN_IS) || check(parser, TOKEN_ISNT) ||
        check(parser, TOKEN_LESS) || check(parser, TOKEN_GREATER) ||
        check(parser, TOKEN_LESSEQUAL) || check(parser, TOKEN_GREATEREQUAL)) {
        
        Token op = peek(parser);
        consume_token(parser);
        
        BinaryExpressionNode* bin = malloc(sizeof(BinaryExpressionNode));
        bin->left = left;
        bin->right = parse_expr(parser);
        if (!bin->right) bin->right = create_dummy_literal();
        
        switch (op) {
            case TOKEN_IS: bin->operator = OP_EQ; break;
            case TOKEN_ISNT: bin->operator = OP_NEQ; break;
            case TOKEN_LESS: bin->operator = OP_LT; break;
            case TOKEN_GREATER: bin->operator = OP_GT; break;
            case TOKEN_LESSEQUAL: bin->operator = OP_LTE; break;
            case TOKEN_GREATEREQUAL: bin->operator = OP_GTE; break;
            default: break;
        }
        
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* left = parse_comparison(parser);
    if (!left) left = create_dummy_literal();
    
    while (check(parser, TOKEN_AND)) {
        consume_token(parser);
        BinaryExpressionNode* bin = malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_AND;
        bin->left = left;
        bin->right = parse_comparison(parser);
        if (!bin->right) bin->right = create_dummy_literal();
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* left = parse_logical_and(parser);
    if (!left) left = create_dummy_literal();
    
    while (check(parser, TOKEN_OR)) {
        consume_token(parser);
        BinaryExpressionNode* bin = malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_OR;
        bin->left = left;
        bin->right = parse_logical_and(parser);
        if (!bin->right) bin->right = create_dummy_literal();
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// ============================================================================
// CENTRAL DISPATCHER
// ============================================================================

/**
 * Main statement parser - routes to appropriate handler
 * GUARANTEED to return non-NULL
 */
ASTNode* parse_statement(Parser* parser) {
    Token tok = peek(parser);
    
    // Skip unexpected EOF gracefully
    if (tok == TOKEN_EOF) {
        return create_dummy_statement();
    }
    
    // Route to appropriate parser
    if (tok == TOKEN_CONST || is_data_type(tok)) {
        return parse_decl_stmt(parser);
    } else if (tok == TOKEN_SHOW) {
        return parse_output_stmt(parser);
    } else if (tok == TOKEN_IF) {
        return parse_cond_stmt(parser);
    } else if (tok == TOKEN_REPEAT) {
        return parse_iter_stmt(parser);
    } else if (tok == TOKEN_RETURN) {
        return parse_return_stmt(parser);
    } else if (tok == TOKEN_CONTINUE) {
        consume_token(parser);
        return create_node(NODE_CONTINUE_STMT, NULL);
    } else if (tok == TOKEN_STOP) {
        consume_token(parser);
        return create_node(NODE_STOP_STMT, NULL);
    } else if (tok == TOKEN_IDENTIFIER) {
        Token next = peek_ahead(parser, 1);
        if (next == TOKEN_ASSIGN || is_assignment_operator(next)) {
            return parse_assign_stmt(parser);
        } else {
            error(parser, "Expected assignment after identifier");
            synchronize_aggressive(parser);
            return create_dummy_statement();
        }
    } else {
        error(parser, "Expected statement");
        synchronize_aggressive(parser);
        return create_dummy_statement();
    }
}
