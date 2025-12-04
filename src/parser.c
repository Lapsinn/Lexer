
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*// Mock token stream for testing: "start number x = 10; show(x + 5); end"
static TokenData mock_tokens[] = {
    {TOKEN_START, "start", {1, 0}, 0},
    {TOKEN_NUMBER, "number", {1, 6}, 0},
    {TOKEN_IDENTIFIER, "x", {1, 13}, 0},
    {TOKEN_ASSIGN, "=", {1, 15}, 0},
    {TOKEN_NUMBER, "10", {1, 17}, 0},
    {TOKEN_SEMICOLON, ";", {1, 19}, 0},
    {TOKEN_SHOW, "show", {1, 21}, 0},
    {TOKEN_LPAREN, "(", {1, 25}, 0},
    {TOKEN_IDENTIFIER, "x", {1, 26}, 0},
    {TOKEN_PLUS, "+", {1, 28}, 0},
    {TOKEN_NUMBER, "5", {1, 30}, 0},
    {TOKEN_RPAREN, ")", {1, 31}, 0},
    {TOKEN_SEMICOLON, ";", {1, 32}, 0},
    {TOKEN_END, "end", {1, 34}, 0},
    {TOKEN_EOF, NULL, {1, 37}, 0}
};
*/
// ============================================================================
// Parser Utilities
// ============================================================================

void consume_token(Parser* parser) {
    if (!is_at_end(parser)) {
        parser->current++;
    }
}

Token peek(Parser* parser) {
    if (parser->current < parser->count) {
        return parser->tokens[parser->current].type;
    }
    return TOKEN_EOF;
}

Token peek_ahead(Parser* parser, int offset) {
    size_t pos = parser->current + offset;
    if (pos < parser->count) {
        return parser->tokens[pos].type;
    }
    return TOKEN_EOF;
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
                tok->loc.line, tok->loc.col, message, 
                tok->val ? tok->val : "<EOF>");
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
// Expression Parsing (Precedence Climbing)
// ============================================================================

// parse_primary: literals, identifiers, parenthesized expressions
ASTNode* parse_primary(Parser* parser) {
    Token tok = peek(parser);
    
    // Number literal
    if (tok == TOKEN_NUMBER || tok == TOKEN_DECIMAL) {
        LiteralNode* lit = (LiteralNode*)malloc(sizeof(LiteralNode));
        if (tok == TOKEN_NUMBER) {
            lit->literal_type = LITERAL_NUMBER;
            lit->value.int_value = atoi(parser->tokens[parser->current].val);
        } else {
            lit->literal_type = LITERAL_DECIMAL;
            lit->value.double_value = atof(parser->tokens[parser->current].val);
        }
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    
    // String literal
    if (tok == TOKEN_STR_LIT) {
        LiteralNode* lit = (LiteralNode*)malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_STRING;
        lit->value.string_value = strdup(parser->tokens[parser->current].val);
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    
    // Character literal
    if (tok == TOKEN_CHAR_LIT) {
        LiteralNode* lit = (LiteralNode*)malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_CHAR;
        lit->value.char_value = parser->tokens[parser->current].val[0];
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    
    // Boolean literals
    if (tok == TOKEN_TRUE || tok == TOKEN_FALSE) {
        LiteralNode* lit = (LiteralNode*)malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_BOOL;
        lit->value.bool_value = (tok == TOKEN_TRUE) ? 1 : 0;
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    
    // NULL literal
    if (tok == TOKEN_NULL) {
        LiteralNode* lit = (LiteralNode*)malloc(sizeof(LiteralNode));
        lit->literal_type = LITERAL_NULL;
        consume_token(parser);
        return create_node(NODE_LITERAL, lit);
    }
    
    // Identifier
    if (tok == TOKEN_IDENTIFIER) {
        IdentifierNode* id = (IdentifierNode*)malloc(sizeof(IdentifierNode));
        id->name = strdup(parser->tokens[parser->current].val);
        consume_token(parser);
        return create_node(NODE_IDENTIFIER, id);
    }
    
    // Parenthesized expression
    if (tok == TOKEN_LPAREN) {
        consume_token(parser);
        ASTNode* expr = parse_expr(parser);
        expect(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    error(parser, "Expected expression");
    return NULL;
}

// parse_unary: handles unary operators (-, +, !)
ASTNode* parse_unary(Parser* parser) {
    Token tok = peek(parser);
    
    if (tok == TOKEN_MIN || tok == TOKEN_PLUS || tok == TOKEN_NOT) {
        consume_token(parser);
        
        UnaryExpressionNode* unary = (UnaryExpressionNode*)malloc(sizeof(UnaryExpressionNode));
        if (tok == TOKEN_MIN) unary->operator = UNOP_NEG;
        else if (tok == TOKEN_PLUS) unary->operator = UNOP_POS;
        else unary->operator = UNOP_NOT;
        
        unary->operand = parse_unary(parser);  // Right associative
        return create_node(NODE_UNARY_EXPR, unary);
    }
    
    return parse_primary(parser);
}

// parse_factor: handles power operator (^) - right associative
ASTNode* parse_factor(Parser* parser) {
    ASTNode* left = parse_unary(parser);
    
    if (check(parser, TOKEN_POW)) {
        consume_token(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_POW;
        bin->left = left;
        bin->right = parse_factor(parser);  // Right associative
        return create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// parse_term: handles *, /, %, ~
ASTNode* parse_term(Parser* parser) {
    ASTNode* left = parse_factor(parser);
    
    while (check(parser, TOKEN_MUL) || check(parser, TOKEN_DIV) || 
           check(parser, TOKEN_MOD) || check(parser, TOKEN_IDIV)) {
        Token op = peek(parser);
        consume_token(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        switch (op) {
            case TOKEN_MUL: bin->operator = OP_MUL; break;
            case TOKEN_DIV: bin->operator = OP_DIV; break;
            case TOKEN_MOD: bin->operator = OP_MOD; break;
            case TOKEN_IDIV: bin->operator = OP_IDIV; break;
            default: break;
        }
        bin->left = left;
        bin->right = parse_factor(parser);
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// parse_expr: handles +, - (also serves as entry point for expressions)
ASTNode* parse_expr(Parser* parser) {
    ASTNode* left = parse_term(parser);
    
    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MIN)) {
        Token op = peek(parser);
        consume_token(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = (op == TOKEN_PLUS) ? OP_ADD : OP_SUB;
        bin->left = left;
        bin->right = parse_term(parser);
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// parse_comparison: handles ==, !=, <, >, <=, >=
ASTNode* parse_comparison(Parser* parser) {
    ASTNode* left = parse_expr(parser);
    
    if (check(parser, TOKEN_IS) || check(parser, TOKEN_ISNT) ||
        check(parser, TOKEN_LESS) || check(parser, TOKEN_GREATER) ||
        check(parser, TOKEN_LESSEQUAL) || check(parser, TOKEN_GREATEREQUAL)) {
        
        Token op = peek(parser);
        consume_token(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        switch (op) {
            case TOKEN_IS: bin->operator = OP_EQ; break;
            case TOKEN_ISNT: bin->operator = OP_NEQ; break;
            case TOKEN_LESS: bin->operator = OP_LT; break;
            case TOKEN_GREATER: bin->operator = OP_GT; break;
            case TOKEN_LESSEQUAL: bin->operator = OP_LTE; break;
            case TOKEN_GREATEREQUAL: bin->operator = OP_GTE; break;
            default: break;
        }
        bin->left = left;
        bin->right = parse_expr(parser);
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// parse_logical_and: handles &&
ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* left = parse_comparison(parser);
    
    while (check(parser, TOKEN_AND)) {
        consume_token(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_AND;
        bin->left = left;
        bin->right = parse_comparison(parser);
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// parse_logical_or: handles ||
ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* left = parse_logical_and(parser);
    
    while (check(parser, TOKEN_OR)) {
        consume_token(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_OR;
        bin->left = left;
        bin->right = parse_logical_and(parser);
        left = create_node(NODE_BINARY_EXPR, bin);
    }
    
    return left;
}

// ============================================================================
// Statement Parsing
// ============================================================================

// parse_decl_stmt: handles variable declarations
// Example: "number x = 10;"
ASTNode* parse_decl_stmt(Parser* parser) {
    int is_const = 0;
    if (check(parser, TOKEN_CONST)) {
        is_const = 1;
        consume_token(parser);
    }
    
    if (!is_data_type(peek(parser))) {
        error(parser, "Expected data type in declaration");
        return NULL;
    }
    
    char* data_type = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected identifier in declaration");
        free(data_type);
        return NULL;
    }
    
    char* var_name = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    ASTNode* init_expr = NULL;
    if (check(parser, TOKEN_ASSIGN)) {
        consume_token(parser);
        init_expr = parse_expr(parser);
    }
    
    VarDeclNode* decl = (VarDeclNode*)malloc(sizeof(VarDeclNode));
    decl->var_name = var_name;
    decl->data_type = data_type;
    decl->init_expr = init_expr;
    decl->is_const = is_const;
    
    return create_node(NODE_VAR_DECL, decl);
}

// parse_assign_stmt: handles assignment statements
// Example: "x = 10;" or "x += 5;"
ASTNode* parse_assign_stmt(Parser* parser) {
    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected identifier in assignment");
        return NULL;
    }
    
    char* var_name = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    if (!is_assignment_operator(peek(parser))) {
        error(parser, "Expected assignment operator");
        free(var_name);
        return NULL;
    }
    
    Token op = peek(parser);
    consume_token(parser);
    
    ASTNode* expr = parse_expr(parser);
    
    AssignStmtNode* assign = (AssignStmtNode*)malloc(sizeof(AssignStmtNode));
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

// parse_output_stmt: handles show statements
// Example: "show(x + 5);"
ASTNode* parse_output_stmt(Parser* parser) {
    expect(parser, TOKEN_SHOW, "Expected 'show'");
    expect(parser, TOKEN_LPAREN, "Expected '(' after 'show'");
    
    ASTNode* expr = parse_expr(parser);
    
    expect(parser, TOKEN_RPAREN, "Expected ')' after expression");
    
    OutputStmtNode* output = (OutputStmtNode*)malloc(sizeof(OutputStmtNode));
    output->expr = expr;
    
    return create_node(NODE_OUTPUT_STMT, output);
}

// parse_input_stmt: handles ask statements
// Example: "x = ask(number);"
ASTNode* parse_input_stmt(Parser* parser) {
    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected identifier before 'ask'");
        return NULL;
    }
    
    char* var_name = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    expect(parser, TOKEN_ASSIGN, "Expected '=' in input statement");
    expect(parser, TOKEN_ASK, "Expected 'ask'");
    expect(parser, TOKEN_LPAREN, "Expected '(' after 'ask'");
    
    if (!is_data_type(peek(parser))) {
        error(parser, "Expected data type in ask statement");
        free(var_name);
        return NULL;
    }
    
    char* data_type = strdup(parser->tokens[parser->current].val);
    consume_token(parser);
    
    expect(parser, TOKEN_RPAREN, "Expected ')' after data type");
    
    InputStmtNode* input = (InputStmtNode*)malloc(sizeof(InputStmtNode));
    input->var_name = var_name;
    input->data_type = data_type;
    
    return create_node(NODE_INPUT_STMT, input);
}

// parse_cond_stmt: handles if-else statements (stub)
ASTNode* parse_cond_stmt(Parser* parser) {
    expect(parser, TOKEN_IF, "Expected 'if'");
    expect(parser, TOKEN_LPAREN, "Expected '(' after 'if'");
    
    ASTNode* condition = parse_logical_or(parser);
    
    expect(parser, TOKEN_RPAREN, "Expected ')' after condition");
    
    // Optional 'then' keyword
    if (check(parser, TOKEN_THEN)) {
        consume_token(parser);
    }
    
    ASTNode* then_body = parse_block(parser);
    
    ASTNode* else_body = NULL;
    if (check(parser, TOKEN_ELSE)) {
        consume_token(parser);
        if (check(parser, TOKEN_IF)) {
            else_body = parse_cond_stmt(parser);
        } else {
            else_body = parse_block(parser);
        }
    }
    
    CondStmtNode* cond = (CondStmtNode*)malloc(sizeof(CondStmtNode));
    cond->condition = condition;
    cond->then_body = then_body;
    cond->else_body = else_body;
    
    return create_node(NODE_COND_STMT, cond);
}

// parse_iter_stmt: handles repeat statements (stub)
ASTNode* parse_iter_stmt(Parser* parser) {
    expect(parser, TOKEN_REPEAT, "Expected 'repeat'");
    expect(parser, TOKEN_LPAREN, "Expected '(' after 'repeat'");
    
    ASTNode* init = parse_statement(parser);
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after init");
    
    ASTNode* condition = parse_logical_or(parser);
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after condition");
    
    ASTNode* increment = parse_statement(parser);
    expect(parser, TOKEN_RPAREN, "Expected ')' after increment");
    
    ASTNode* body = parse_block(parser);
    
    IterStmtNode* iter = (IterStmtNode*)malloc(sizeof(IterStmtNode));
    iter->init = init;
    iter->condition = condition;
    iter->increment = increment;
    iter->body = body;
    
    return create_node(NODE_ITER_STMT, iter);
}

// parse_return_stmt: handles return statements
ASTNode* parse_return_stmt(Parser* parser) {
    expect(parser, TOKEN_RETURN, "Expected 'return'");
    
    ASTNode* expr = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) {
        expr = parse_expr(parser);
    }
    
    ReturnStmtNode* ret = (ReturnStmtNode*)malloc(sizeof(ReturnStmtNode));
    ret->expr = expr;
    
    return create_node(NODE_RETURN_STMT, ret);
}

// parse_block: handles { statement_list }
ASTNode* parse_block(Parser* parser) {
    expect(parser, TOKEN_LBRACE, "Expected '{' to start block");
    
    ASTNode* stmt_list = parse_statement_list(parser);
    
    expect(parser, TOKEN_RBRACE, "Expected '}' to end block");
    
    return create_node(NODE_BLOCK, stmt_list);
}

// parse_statement: dispatches to specific statement parsers
ASTNode* parse_statement(Parser* parser) {
    Token tok = peek(parser);
    
    // Check for declarations
    if (tok == TOKEN_CONST || is_data_type(tok)) {
        return parse_decl_stmt(parser);
    }
    
    // Check for show statement
    if (tok == TOKEN_SHOW) {
        return parse_output_stmt(parser);
    }
    
    // Check for if statement
    if (tok == TOKEN_IF) {
        return parse_cond_stmt(parser);
    }
    
    // Check for repeat statement
    if (tok == TOKEN_REPEAT) {
        return parse_iter_stmt(parser);
    }
    
    // Check for return statement
    if (tok == TOKEN_RETURN) {
        return parse_return_stmt(parser);
    }
    
    // Check for continue statement
    if (tok == TOKEN_CONTINUE) {
        consume_token(parser);
        return create_node(NODE_CONTINUE_STMT, NULL);
    }
    
    // Check for stop statement
    if (tok == TOKEN_STOP) {
        consume_token(parser);
        return create_node(NODE_STOP_STMT, NULL);
    }
    
    // Check for assignment or input (both start with identifier)
    if (tok == TOKEN_IDENTIFIER) {
        Token next = peek_ahead(parser, 1);
        if (next == TOKEN_ASSIGN && peek_ahead(parser, 2) == TOKEN_ASK) {
            return parse_input_stmt(parser);
        } else if (is_assignment_operator(next)) {
            return parse_assign_stmt(parser);
        }
    }
    
    error(parser, "Unknown statement");
    return NULL;
}

// parse_statement_list: parses a list of statements
ASTNode* parse_statement_list(Parser* parser) {
    StatementListNode* list = (StatementListNode*)malloc(sizeof(StatementListNode));
    list->statements = NULL;
    list->count = 0;
    list->capacity = 0;
    
    while (!check(parser, TOKEN_END) && !check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            add_statement(list, stmt);
        }
        
        // Expect semicolon after statement (except for blocks)
        if (stmt && stmt->type != NODE_COND_STMT && stmt->type != NODE_ITER_STMT) {
            expect(parser, TOKEN_SEMICOLON, "Expected ';' after statement");
        }
        
        if (parser->has_error) break;
    }
    
    return create_node(NODE_STATEMENT_LIST, list);
}

// parse_program: main entry point
// Program => TOKEN_START <STMT_LIST> TOKEN_END
ASTNode* parse_program(Parser* parser) {
    expect(parser, TOKEN_START, "Expected 'start' at beginning of program");
    
    ASTNode* stmt_list = parse_statement_list(parser);
    
    expect(parser, TOKEN_END, "Expected 'end' at end of program");
    
    return create_node(NODE_PROGRAM, stmt_list);
}

