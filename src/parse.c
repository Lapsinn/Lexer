#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Internal Helpers
// ============================================================================

static const char* get_token_type_name(Token type);

// ============================================================================
// Parser Initialization & Core
// ============================================================================

Parser* parser_create(TokenData* tokens, size_t count) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for Parser\n");
        return NULL;
    }
    parser->tokens = tokens;
    parser->current = 0;
    parser->count = count;
    parser->has_error = 0;
    return parser;
}

void parser_destroy(Parser* parser) {
    if (parser) free(parser);
}

int is_at_end(Parser* parser) {
    if (!parser) return 1;
    return parser->current >= parser->count || 
           parser->tokens[parser->current].type == TOKEN_EOF; 
}

TokenData peek(Parser* parser) {
    if (!parser || parser->current >= parser->count) 
        return (TokenData){TOKEN_EOF, NULL, {0, 0}, 0};
    return parser->tokens[parser->current];
}

TokenData peek_ahead(Parser* parser, int offset) {
    if (!parser) return (TokenData){TOKEN_EOF, NULL, {0, 0}, 0};
    
    size_t pos = parser->current + offset;
    if (pos >= parser->count) {
        return (TokenData){TOKEN_EOF, NULL, {0, 0}, 0};
    }
    
    return parser->tokens[pos];
}

TokenData previous(Parser* parser) {
    if (!parser || parser->current == 0) return (TokenData){TOKEN_EOF, NULL, {0, 0}, 0};
    return parser->tokens[parser->current - 1];
}

void read_token(Parser* parser) {
    if (!is_at_end(parser)) {
        parser->current++;
    }
}

int check(Parser* parser, Token type) {
    if (is_at_end(parser)) return 0;
    return peek(parser).type == type;
}

int expect(Parser* parser, Token type, const char* message) {
    if (check(parser, type)) {
        read_token(parser);
        return 1;
    }
    error(parser, message);
    return 0;
}

void error(Parser* parser, const char* message) {
    if (parser->has_error) return; 
    parser->has_error = 1;
    TokenData token = peek(parser);
    
    const char* type_name = get_token_type_name(token.type);
    fprintf(stderr, "[line %zu] Error at '%s' (%s): %s\n", 
            token.loc.line, 
            token.val ? token.val : "EOF", 
            type_name,
            message);
}

// ============================================================================
// Panic Mode Recovery
// ============================================================================

int is_statement_start(Token token) {
    switch (token) {
        case TOKEN_CONST:
        case TOKEN_NUMBER: case TOKEN_DECIMAL: case TOKEN_LETTER: case TOKEN_BOOL: 
        case TOKEN_IDENTIFIER:
        case TOKEN_SHOW:
        case TOKEN_IF:
        case TOKEN_REPEAT:
        case TOKEN_RETURN:
        case TOKEN_CONTINUE:
        case TOKEN_STOP:
        case TOKEN_LBRACE:
            return 1;
        default: return 0;
    }
}

void synchronize(Parser* parser) {
    parser->has_error = 0;

    while (!is_at_end(parser) && peek(parser).type != TOKEN_EOF) {
        if (previous(parser).type == TOKEN_SEMICOLON) return;
        if (previous(parser).type == TOKEN_RBRACE) return;
        if (is_statement_start(peek(parser).type)) return;
        read_token(parser);
    }
}

// ============================================================================
// Grammar Implementation: Expressions (Bottom-Up)
// ============================================================================

ASTNode* parse_expr(Parser* parser);

ASTNode* parse_value(Parser* parser) {
    TokenData t = peek(parser);
    
    if (t.type == TOKEN_IDENTIFIER) {
        read_token(parser);
        IdentifierNode* id = (IdentifierNode*)malloc(sizeof(IdentifierNode));
        id->name = strdup(t.val);
        return create_node_with_loc(NODE_IDENTIFIER, id, t);
    }
    
    LiteralNode* lit = (LiteralNode*)malloc(sizeof(LiteralNode));

    switch (t.type) {
        case TOKEN_INTEGER:
            lit->literal_type = LITERAL_NUMBER;
            lit->value.int_value = atoi(t.val);
            read_token(parser);
            break;
        case TOKEN_FLOAT:
            lit->literal_type = LITERAL_DECIMAL;
            lit->value.double_value = atof(t.val);
            read_token(parser);
            break;
        case TOKEN_STR_LIT:
            lit->literal_type = LITERAL_STRING;
            lit->value.string_value = strdup(t.val);
            read_token(parser);
            break;
        case TOKEN_CHAR_LIT:
            lit->literal_type = LITERAL_CHAR;
            lit->value.char_value = t.val[0];
            read_token(parser);
            break;
        case TOKEN_TRUE:
            lit->literal_type = LITERAL_BOOL;
            lit->value.bool_value = 1;
            read_token(parser);
            break;
        case TOKEN_FALSE:
            lit->literal_type = LITERAL_BOOL;
            lit->value.bool_value = 0;
            read_token(parser);
            break;
        case TOKEN_NULL:
            lit->literal_type = LITERAL_NULL;
            read_token(parser);
            break;
        default:
            free(lit);
            return NULL; 
    }
    
    return create_node_with_loc(NODE_LITERAL, lit, t);
}

ASTNode* parse_arg_list(Parser* parser) {
    ArgListNode* list = (ArgListNode*)malloc(sizeof(ArgListNode));
    list->count = 0;
    list->capacity = 4;
    list->args = (ASTNode**)malloc(sizeof(ASTNode*) * list->capacity);

    do {
        ASTNode* expr = parse_expr(parser);
        if (!expr) {
            error(parser, "Expected expression in argument list");
            break;
        }

        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->args = (ASTNode**)realloc(list->args, sizeof(ASTNode*) * list->capacity);
        }
        list->args[list->count++] = expr;

    } while (check(parser, TOKEN_COMMA) && (read_token(parser), 1));

    return create_node(NODE_ARG_LIST, list);
}

ASTNode* parse_fn_call(Parser* parser) {
    TokenData id_tok = peek(parser);
    read_token(parser); // ID
    read_token(parser); // '('

    FuncCallNode* call = (FuncCallNode*)malloc(sizeof(FuncCallNode));
    call->func_name = strdup(id_tok.val);
    call->arguments = NULL;
    call->count = 0;
    call->capacity = 0;

    if (!check(parser, TOKEN_RPAREN)) {
        ASTNode* argsNode = parse_arg_list(parser);
        if (argsNode) {
            ArgListNode* list = (ArgListNode*)argsNode->specific_node;
            call->count = list->count;
            call->capacity = list->capacity;
            call->arguments = list->args;
            free(list); 
            free(argsNode); 
        }
    }

    if (!expect(parser, TOKEN_RPAREN, "Expected ')' after function call arguments")) {
        return NULL;
    }
    return create_node_with_loc(NODE_FUNC_CALL, call, id_tok);
}

ASTNode* parse_input_expr(Parser* parser) {
    TokenData ask_tok = peek(parser);
    read_token(parser); // ask
    
    if (!expect(parser, TOKEN_LPAREN, "Expected '(' after 'ask'")) return NULL;
    
    if (!is_data_type(peek(parser).type)) {
        error(parser, "Expected data type in ask expression");
        return NULL;
    }
    
    TokenData type_tok = peek(parser);
    read_token(parser); // Type
    
    if (!expect(parser, TOKEN_RPAREN, "Expected ')' after type")) return NULL;

    InputStmtNode* input = (InputStmtNode*)malloc(sizeof(InputStmtNode));
    input->data_type = strdup(type_tok.val);
    input->var_name = NULL; 

    return create_node_with_loc(NODE_INPUT_STMT, input, ask_tok);
}

ASTNode* parse_sizeof(Parser* parser) {
    TokenData tok = peek(parser);
    read_token(parser); 
    if (!expect(parser, TOKEN_LPAREN, "Expected '('")) return NULL;
    ASTNode* val = parse_value(parser);
    if (!val) {
        error(parser, "Expected value for sizeof");
        return NULL;
    }
    if (!expect(parser, TOKEN_RPAREN, "Expected ')'")) return NULL;
    
    UnaryExpressionNode* unop = (UnaryExpressionNode*)malloc(sizeof(UnaryExpressionNode));
    unop->operator = UNOP_POS; 
    unop->operand = val;
    return create_node_with_loc(NODE_UNARY_EXPR, unop, tok);
}

ASTNode* parse_exp_rule(Parser* parser) {
    if (check(parser, TOKEN_LPAREN)) {
        read_token(parser);
        ASTNode* expr = parse_expr(parser);
        if (!expect(parser, TOKEN_RPAREN, "Expected ')' after expression")) return NULL;
        return expr;
    }
    
    if (check(parser, TOKEN_SIZEOF)) return parse_sizeof(parser);
    if (check(parser, TOKEN_ASK)) return parse_input_expr(parser);
    
    if (check(parser, TOKEN_IDENTIFIER)) {
        TokenData next = peek_ahead(parser, 1);
        if (next.type == TOKEN_LPAREN) {
            return parse_fn_call(parser);
        }
    }
    
    ASTNode* val = parse_value(parser);
    if (val) return val;

    error(parser, "Expected expression (value, call, or parenthesized group)");
    return NULL;
}

ASTNode* parse_unary(Parser* parser) {
    if (check(parser, TOKEN_NOT) || check(parser, TOKEN_MIN) || check(parser, TOKEN_PLUS)) {
        TokenData op_tok = peek(parser);
        int op = map_unary_operator(op_tok.type);
        read_token(parser);
        
        ASTNode* operand = parse_unary(parser);
        UnaryExpressionNode* unop = (UnaryExpressionNode*)malloc(sizeof(UnaryExpressionNode));
        unop->operator = op;
        unop->operand = operand;
        return create_node_with_loc(NODE_UNARY_EXPR, unop, op_tok);
    }
    return parse_exp_rule(parser);
}

ASTNode* parse_factor(Parser* parser) {
    ASTNode* left = parse_unary(parser);
    
    if (check(parser, TOKEN_POW)) {
        TokenData op_tok = peek(parser);
        read_token(parser);
        ASTNode* right = parse_factor(parser); 
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_POW;
        bin->left = left;
        bin->right = right;
        return create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* left = parse_factor(parser);
    
    while (check(parser, TOKEN_MUL) || check(parser, TOKEN_DIV) || 
           check(parser, TOKEN_MOD) || check(parser, TOKEN_IDIV)) {
        TokenData op_tok = peek(parser);
        int op = map_binary_operator(op_tok.type);
        read_token(parser);
        ASTNode* right = parse_factor(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = op;
        bin->left = left;
        bin->right = right;
        left = create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_arith_expr(Parser* parser) {
    ASTNode* left = parse_term(parser);
    
    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MIN)) {
        TokenData op_tok = peek(parser);
        int op = map_binary_operator(op_tok.type);
        read_token(parser);
        ASTNode* right = parse_term(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = op;
        bin->left = left;
        bin->right = right;
        left = create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_relational(Parser* parser) {
    ASTNode* left = parse_arith_expr(parser);
    
    while (is_relational_operator(peek(parser).type)) {
        TokenData op_tok = peek(parser);
        int op = map_binary_operator(op_tok.type);
        read_token(parser);
        ASTNode* right = parse_arith_expr(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = op;
        bin->left = left;
        bin->right = right;
        left = create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_equality(Parser* parser) {
    ASTNode* left = parse_relational(parser);
    
    while (check(parser, TOKEN_IS) || check(parser, TOKEN_ISNT)) {
        TokenData op_tok = peek(parser);
        int op = map_binary_operator(op_tok.type);
        read_token(parser);
        ASTNode* right = parse_relational(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = op;
        bin->left = left;
        bin->right = right;
        left = create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_bool_and(Parser* parser) {
    ASTNode* left = parse_equality(parser);
    
    while (check(parser, TOKEN_AND)) {
        TokenData op_tok = peek(parser);
        read_token(parser);
        ASTNode* right = parse_equality(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_AND;
        bin->left = left;
        bin->right = right;
        left = create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_bool_or(Parser* parser) {
    ASTNode* left = parse_bool_and(parser);
    
    while (check(parser, TOKEN_OR)) {
        TokenData op_tok = peek(parser);
        read_token(parser);
        ASTNode* right = parse_bool_and(parser);
        
        BinaryExpressionNode* bin = (BinaryExpressionNode*)malloc(sizeof(BinaryExpressionNode));
        bin->operator = OP_OR;
        bin->left = left;
        bin->right = right;
        left = create_node_with_loc(NODE_BINARY_EXPR, bin, op_tok);
    }
    return left;
}

ASTNode* parse_expr(Parser* parser) {
    return parse_bool_or(parser);
}

// ============================================================================
// Statements
// ============================================================================

ASTNode* parse_statement(Parser* parser);
ASTNode* parse_stmt_list(Parser* parser, Token delimiter);
ASTNode* parse_block(Parser* parser);

ASTNode* parse_param_list(Parser* parser) {
    ParameterListNode* list = (ParameterListNode*)malloc(sizeof(ParameterListNode));
    list->count = 0;
    list->capacity = 4;
    list->parameters = (ParameterNode**)malloc(sizeof(ParameterNode*) * list->capacity);

    do {
        if (!is_data_type(peek(parser).type)) {
            error(parser, "Expected parameter type");
            break;
        }
        char* type = strdup(peek(parser).val);
        read_token(parser);

        if (!check(parser, TOKEN_IDENTIFIER)) {
            error(parser, "Expected parameter name");
            free(type);
            break;
        }
        char* name = strdup(peek(parser).val);
        read_token(parser);

        ParameterNode* param = (ParameterNode*)malloc(sizeof(ParameterNode));
        param->param_type = type;
        param->param_name = name;

        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->parameters = (ParameterNode**)realloc(list->parameters, sizeof(ParameterNode*) * list->capacity);
        }
        list->parameters[list->count++] = param;

    } while (check(parser, TOKEN_COMMA) && (read_token(parser), 1));

    return create_node(NODE_PARAM_LIST, list);
}

ASTNode* parse_func_decl(Parser* parser) {
    TokenData type_tok = peek(parser);
    read_token(parser); 
    
    if (!expect(parser, TOKEN_IDENTIFIER, "Expected function name")) {
        return NULL;
    }
    TokenData id_tok = previous(parser);
    
    if (!expect(parser, TOKEN_LPAREN, "Expected '('")) {
        return NULL;
    }
    
    ASTNode* params = NULL;
    if (!check(parser, TOKEN_RPAREN)) {
        params = parse_param_list(parser);
    }
    
    if (!expect(parser, TOKEN_RPAREN, "Expected ')'")) {
        return NULL;
    }
    
    ASTNode* body = parse_block(parser);
    
    FuncDeclNode* func = (FuncDeclNode*)malloc(sizeof(FuncDeclNode));
    func->return_type = strdup(type_tok.val);
    func->func_name = strdup(id_tok.val);
    func->params = params;
    func->body = body;
    
    return create_node_with_loc(NODE_FUNC_DECL, func, type_tok);
}

ASTNode* parse_single_decl(Parser* parser, char* data_type, int is_const) {
    TokenData id_tok = peek(parser);
    
    if (!expect(parser, TOKEN_IDENTIFIER, "Expected variable name")) {
        return NULL; 
    }
    
    ASTNode* init = NULL;
    if (check(parser, TOKEN_ASSIGN)) {
        read_token(parser);
        init = parse_expr(parser);
    }
    
    VarDeclNode* decl = (VarDeclNode*)malloc(sizeof(VarDeclNode));
    decl->var_name = strdup(id_tok.val);
    decl->data_type = strdup(data_type);
    decl->is_const = is_const;
    decl->init_expr = init;
    
    return create_node_with_loc(NODE_VAR_DECL, decl, id_tok);
}

ASTNode* parse_var_decl_wrapper(Parser* parser, int is_const) {
    TokenData type_tok = peek(parser);
    char* data_type = strdup(type_tok.val);
    read_token(parser);
    
    StatementListNode* list = (StatementListNode*)malloc(sizeof(StatementListNode));
    list->count = 0;
    list->capacity = 4;
    list->statements = (ASTNode**)malloc(sizeof(ASTNode*) * list->capacity);
    
    do {
        ASTNode* decl = parse_single_decl(parser, data_type, is_const);
        if (!decl) {
            free(data_type);
            free(list->statements);
            free(list);
            return NULL;
        }

        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->statements = (ASTNode**)realloc(list->statements, sizeof(ASTNode*) * list->capacity);
        }
        list->statements[list->count++] = decl;
    } while (check(parser, TOKEN_COMMA) && (read_token(parser), 1));
    
    free(data_type);
    
    if (list->count == 1) {
        ASTNode* single = list->statements[0];
        free(list->statements);
        free(list);
        return single;
    }
    
    return create_node(NODE_STATEMENT_LIST, list);
}

ASTNode* parse_decl_stmt(Parser* parser) {
    int is_const = 0;
    if (check(parser, TOKEN_CONST)) {
        read_token(parser);
        is_const = 1;
        return parse_var_decl_wrapper(parser, is_const);
    }
    
    TokenData type_tok = peek(parser);
    if (!is_data_type(type_tok.type)) {
        error(parser, "Expected declaration");
        return NULL;
    }
    
    TokenData next = peek_ahead(parser, 1);
    if (next.type == TOKEN_IDENTIFIER) {
        TokenData after = peek_ahead(parser, 2);
        if (after.type == TOKEN_LPAREN) {
            return parse_func_decl(parser);
        }
    }
    
    return parse_var_decl_wrapper(parser, is_const);
}

ASTNode* parse_ass_stmt(Parser* parser) {
    TokenData id_tok = peek(parser);
    read_token(parser); // ID
    
    if (!is_assignment_operator(peek(parser).type)) {
        error(parser, "Expected assignment operator");
        return NULL;
    }
    
    int op = map_assign_operator(peek(parser).type);
    read_token(parser);
    
    ASTNode* expr = parse_expr(parser);
    
    AssignStmtNode* assign = (AssignStmtNode*)malloc(sizeof(AssignStmtNode));
    assign->var_name = strdup(id_tok.val);
    assign->assign_type = op;
    assign->expr = expr;
    
    return create_node_with_loc(NODE_ASSIGN_STMT, assign, id_tok);
}

ASTNode* parse_input_stmt(Parser* parser) {
    TokenData id_tok = peek(parser);
    read_token(parser); // ID
    
    if (!expect(parser, TOKEN_ASSIGN, "Expected '='")) return NULL;
    
    ASTNode* inputNode = parse_input_expr(parser);
    if (!inputNode) return NULL;

    InputStmtNode* data = (InputStmtNode*)inputNode->specific_node;
    data->var_name = strdup(id_tok.val);
    
    return inputNode;
}

ASTNode* parse_output_stmt(Parser* parser) {
    TokenData tok = peek(parser);
    read_token(parser); // SHOW
    if (!expect(parser, TOKEN_LPAREN, "Expected '('")) return NULL;
    ASTNode* expr = parse_expr(parser);
    if (!expect(parser, TOKEN_RPAREN, "Expected ')'")) return NULL;
    
    OutputStmtNode* out = (OutputStmtNode*)malloc(sizeof(OutputStmtNode));
    out->expr = expr;
    return create_node_with_loc(NODE_OUTPUT_STMT, out, tok);
}

ASTNode* parse_block(Parser* parser) {
    TokenData tok = peek(parser);
    if (!expect(parser, TOKEN_LBRACE, "Expected '{'")) return NULL;
    
    ASTNode* list = NULL;
    if (!check(parser, TOKEN_RBRACE)) {
        list = parse_stmt_list(parser, TOKEN_RBRACE);
    }
    
    if (!expect(parser, TOKEN_RBRACE, "Expected '}'")) return NULL;
    
    StatementListNode* blkData = list ? (StatementListNode*)list->specific_node : NULL;
    if (!blkData) {
        blkData = (StatementListNode*)malloc(sizeof(StatementListNode));
        blkData->count=0; blkData->capacity=0; blkData->statements=NULL;
    }
    
    if (list) free(list); 
    
    return create_node_with_loc(NODE_BLOCK, blkData, tok);
}

ASTNode* parse_cond_stmt(Parser* parser) {
    TokenData tok = peek(parser);
    read_token(parser); // IF
    if (!expect(parser, TOKEN_LPAREN, "Expected '('")) return NULL;
    ASTNode* cond = parse_expr(parser);
    if (!expect(parser, TOKEN_RPAREN, "Expected ')'")) return NULL;
    
    if (check(parser, TOKEN_THEN)) read_token(parser);
    
    ASTNode* thenBody = NULL;
    if (check(parser, TOKEN_LBRACE)) {
        thenBody = parse_block(parser);
    } else {
        thenBody = parse_statement(parser); 
    }
    
    ASTNode* elseBody = NULL;
    if (check(parser, TOKEN_ELSE)) {
        read_token(parser);
        if (check(parser, TOKEN_IF)) {
            elseBody = parse_cond_stmt(parser); 
        } else if (check(parser, TOKEN_LBRACE)) {
            elseBody = parse_block(parser);
        } else {
            elseBody = parse_statement(parser);
        }
    }
    
    CondStmtNode* node = (CondStmtNode*)malloc(sizeof(CondStmtNode));
    node->condition = cond;
    node->then_body = thenBody;
    node->else_body = elseBody;
    return create_node_with_loc(NODE_COND_STMT, node, tok);
}

ASTNode* parse_iter_stmt(Parser* parser) {
    TokenData tok = peek(parser);
    read_token(parser); // REPEAT
    if (!expect(parser, TOKEN_LPAREN, "Expected '('")) return NULL;
    
    ASTNode* init = NULL;
    if (is_data_type(peek(parser).type) || check(parser, TOKEN_CONST)) {
        int is_c = check(parser, TOKEN_CONST) ? (read_token(parser), 1) : 0;
        init = parse_var_decl_wrapper(parser, is_c);
    } else {
        init = parse_ass_stmt(parser);
    }
    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' after init")) return NULL;
    
    ASTNode* cond = parse_expr(parser);
    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' after condition")) return NULL;
    
    ASTNode* inc = parse_ass_stmt(parser);
    if (!expect(parser, TOKEN_RPAREN, "Expected ')'")) return NULL;

    ASTNode* body = parse_block(parser);
    
    IterStmtNode* iter = (IterStmtNode*)malloc(sizeof(IterStmtNode));
    iter->init = init;
    iter->condition = cond;
    iter->increment = inc;
    iter->body = body;
    return create_node_with_loc(NODE_ITER_STMT, iter, tok);
}

ASTNode* parse_return_stmt(Parser* parser) {
    TokenData tok = peek(parser);
    read_token(parser);
    ASTNode* expr = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) {
        expr = parse_expr(parser);
    }
    ReturnStmtNode* ret = (ReturnStmtNode*)malloc(sizeof(ReturnStmtNode));
    ret->expr = expr;
    return create_node_with_loc(NODE_RETURN_STMT, ret, tok);
}

ASTNode* parse_simple_stmt(Parser* parser) {
    Token t = peek(parser).type;
    
    if (t == TOKEN_CONST || is_data_type(t)) return parse_decl_stmt(parser);
    if (t == TOKEN_SHOW) return parse_output_stmt(parser);
    if (t == TOKEN_RETURN) return parse_return_stmt(parser);
    if (t == TOKEN_CONTINUE) { read_token(parser); return create_node(NODE_CONTINUE_STMT, NULL); }
    if (t == TOKEN_STOP) { read_token(parser); return create_node(NODE_STOP_STMT, NULL); }
    
    if (t == TOKEN_IDENTIFIER) {
        TokenData next = peek_ahead(parser, 1);
        if (next.type == TOKEN_LPAREN) return parse_fn_call(parser);
        
        if (next.type == TOKEN_ASSIGN) {
            TokenData next2 = peek_ahead(parser, 2);
            if (next2.type == TOKEN_ASK) return parse_input_stmt(parser);
        }
        
        return parse_ass_stmt(parser);
    }
    
    // Safe Panic: Consume bad token so we don't loop
    error(parser, "Unknown simple statement");
    read_token(parser); 
    return NULL;
}

ASTNode* parse_compound_stmt(Parser* parser) {
    if (check(parser, TOKEN_IF)) return parse_cond_stmt(parser);
    if (check(parser, TOKEN_REPEAT)) return parse_iter_stmt(parser);
    if (check(parser, TOKEN_LBRACE)) return parse_block(parser);
    return NULL;
}

ASTNode* parse_statement(Parser* parser) {
    if (check(parser, TOKEN_IF) || check(parser, TOKEN_REPEAT) || check(parser, TOKEN_LBRACE)) {
        return parse_compound_stmt(parser);
    }
    
    if (check(parser, TOKEN_SEMICOLON)) {
        read_token(parser);
        return NULL; 
    }
    
    ASTNode* node = parse_simple_stmt(parser);
    
    if (node) expect(parser, TOKEN_SEMICOLON, "Expected ';' after statement");
    
    return node;
}

ASTNode* parse_stmt_list(Parser* parser, Token delimiter) {
    StatementListNode* list = (StatementListNode*)malloc(sizeof(StatementListNode));
    list->count = 0; list->capacity = 8;
    list->statements = (ASTNode**)malloc(sizeof(ASTNode*) * list->capacity);
    
    while (!is_at_end(parser) && !check(parser, delimiter)) {
        size_t start_pos = parser->current; 
        
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            add_statement(list, stmt);
        } else {
            if (parser->has_error) {
                synchronize(parser);
                if (parser->current == start_pos && !is_at_end(parser)) {
                    read_token(parser);
                }
            }
        }
    }
    
    return create_node(NODE_STATEMENT_LIST, list);
}

ASTNode* parse_global_declaration(Parser* parser) {
    int is_const = 0;
    if (check(parser, TOKEN_CONST)) {
        is_const = 1;
        read_token(parser); 
        ASTNode* node = parse_var_decl_wrapper(parser, is_const);
        if (node) expect(parser, TOKEN_SEMICOLON, "Expected ';'"); 
        return node;
    }
    
    if (!is_data_type(peek(parser).type)) {
        error(parser, "Expected global declaration");
        return NULL;
    }
    
    TokenData next = peek_ahead(parser, 1);
    if (next.type == TOKEN_IDENTIFIER) {
        TokenData after = peek_ahead(parser, 2);
        if (after.type == TOKEN_LPAREN) {
            return parse_func_decl(parser);
        }
    }
    
    ASTNode* node = parse_var_decl_wrapper(parser, is_const);
    if (node) expect(parser, TOKEN_SEMICOLON, "Expected ';'");
    return node;
}

ASTNode* parse_program(Parser* parser) {
    ProgramNode* prog = (ProgramNode*)malloc(sizeof(ProgramNode));
    prog->global_count = 0;
    prog->global_capacity = 8;
    prog->global_decls = (ASTNode**)malloc(sizeof(ASTNode*) * prog->global_capacity);
    
    while (!check(parser, TOKEN_START) && !is_at_end(parser)) {
        ASTNode* decl = parse_global_declaration(parser);
        if (decl) {
            add_global_decl(prog, decl);
        } else {
            // FIX: If we have an error, we manually reset the error flag if we successfully sync.
            // We use a custom sync loop here because global sync points are different (semicolon, start).
            
            Token t = peek(parser).type;
            int synced = 0;
            
            // Read until we find a semicolon (end of bad stmt) or a new start (type/const/start)
            while (t != TOKEN_EOF) {
                if (t == TOKEN_SEMICOLON) {
                    read_token(parser); // Eat the semi
                    synced = 1;
                    break;
                }
                if (is_data_type(t) || t == TOKEN_CONST || t == TOKEN_START) {
                    // Don't read; let the loop retry parsing this valid token
                    synced = 1;
                    break;
                }
                read_token(parser);
                t = peek(parser).type;
            }
            
            if (synced) {
                parser->has_error = 0; // Clear flag to allow new errors
            }
        }
    }
    
    expect(parser, TOKEN_START, "Expected 'start'");

    prog->stmt_list = parse_stmt_list(parser, TOKEN_END);
    
    expect(parser, TOKEN_END, "Expected 'end'");
    
    return create_node(NODE_PROGRAM, prog);
}

// ============================================================================
// Helper Functions - Token Type Checking
// ============================================================================

int is_data_type(Token token) {
    switch (token) {
        case TOKEN_NUMBER:
        case TOKEN_DECIMAL:
        case TOKEN_LETTER:
        case TOKEN_STR_LIT:
        case TOKEN_BOOL:
            return 1;
        default:
            return 0;
    }
}

int is_assignment_operator(Token token) {
    switch (token) {
        case TOKEN_ASSIGN:
        case TOKEN_PLUS_EQUAL:
        case TOKEN_MINUS_EQUAL:
        case TOKEN_MUL_EQUAL:
        case TOKEN_DIV_EQUAL:
        case TOKEN_MOD_EQUAL:
        case TOKEN_IDIV_EQUAL:
            return 1;
        default:
            return 0;
    }
}

int is_relational_operator(Token token) {
    switch (token) {
        case TOKEN_IS:
        case TOKEN_ISNT:
        case TOKEN_GREATER:
        case TOKEN_LESS:
        case TOKEN_GREATEREQUAL:
        case TOKEN_LESSEQUAL:
            return 1;
        default:
            return 0;
    }
}

// ============================================================================
// Operator Mappings
// ============================================================================

int map_assign_operator(Token token) {
    switch (token) {
        case TOKEN_ASSIGN:       return ASSIGN_DIRECT;
        case TOKEN_PLUS_EQUAL:   return ASSIGN_PLUS;
        case TOKEN_MINUS_EQUAL:  return ASSIGN_MINUS;
        case TOKEN_MUL_EQUAL:    return ASSIGN_MUL;
        case TOKEN_DIV_EQUAL:    return ASSIGN_DIV;
        case TOKEN_MOD_EQUAL:    return ASSIGN_MOD;
        default:                 return ASSIGN_DIRECT;
    }
}

int map_binary_operator(Token token) {
    switch (token) {
        case TOKEN_PLUS:         return OP_ADD;
        case TOKEN_MIN:          return OP_SUB;
        case TOKEN_MUL:          return OP_MUL;
        case TOKEN_DIV:          return OP_DIV;
        case TOKEN_MOD:          return OP_MOD;
        case TOKEN_IDIV:         return OP_IDIV;
        case TOKEN_POW:          return OP_POW;
        case TOKEN_IS:           return OP_EQ;
        case TOKEN_ISNT:         return OP_NEQ;
        case TOKEN_GREATER:      return OP_GT;
        case TOKEN_LESS:         return OP_LT;
        case TOKEN_GREATEREQUAL: return OP_GTE;
        case TOKEN_LESSEQUAL:    return OP_LTE;
        case TOKEN_AND:          return OP_AND;
        case TOKEN_OR:           return OP_OR;
        default:                 return -1;
    }
}

int map_unary_operator(Token token) {
    switch (token) {
        case TOKEN_MIN:          return UNOP_NEG;
        case TOKEN_NOT:          return UNOP_NOT;
        case TOKEN_PLUS:         return UNOP_POS;
        default:                 return -1;
    }
}

// ============================================================================
// Internal Helper: Token Name Lookup
// ============================================================================

static const char* get_token_type_name(Token type) {
    switch (type) {
        case TOKEN_IDENTIFIER:      return "IDENTIFIER";
        case TOKEN_INTEGER:         return "INTEGER";
        case TOKEN_FLOAT:           return "FLOAT";
        case TOKEN_STR_LIT:         return "STRING_LITERAL";
        case TOKEN_CHAR_LIT:        return "CHAR_LITERAL";
        case TOKEN_NUMBER:          return "number";
        case TOKEN_DECIMAL:         return "decimal";
        case TOKEN_LETTER:          return "letter";
        case TOKEN_BOOL:            return "bool";
        case TOKEN_WORD:            return "word";
        case TOKEN_TRUE:            return "true";
        case TOKEN_FALSE:           return "false";
        case TOKEN_NULL:            return "null";
        case TOKEN_CONST:           return "const";
        case TOKEN_SIZEOF:          return "sizeof";
        case TOKEN_START:           return "start";
        case TOKEN_END:             return "end";
        case TOKEN_IF:              return "if";
        case TOKEN_ELSE:            return "else";
        case TOKEN_THEN:            return "then";
        case TOKEN_REPEAT:          return "repeat";
        case TOKEN_WHILE:           return "while";
        case TOKEN_LOOP:            return "loop";
        case TOKEN_CONTINUE:        return "continue";
        case TOKEN_STOP:            return "stop";
        case TOKEN_RETURN:          return "return";
        case TOKEN_GOTO:            return "goto";
        case TOKEN_EXIT:            return "exit";
        case TOKEN_MAIN:            return "main";
        case TOKEN_IMPORT:          return "import";
        case TOKEN_SHOW:            return "show";
        case TOKEN_ASK:             return "ask";
        case TOKEN_EACH:            return "each";
        case TOKEN_OF:              return "of";
        case TOKEN_ALSO:            return "also";
        case TOKEN_PLUS:            return "+";
        case TOKEN_MIN:             return "-";
        case TOKEN_MUL:             return "*";
        case TOKEN_DIV:             return "/";
        case TOKEN_MOD:             return "%";
        case TOKEN_IDIV:            return "~";
        case TOKEN_POW:             return "^";
        case TOKEN_IS:              return "==";
        case TOKEN_ISNT:            return "!=";
        case TOKEN_GREATER:         return ">";
        case TOKEN_LESS:            return "<";
        case TOKEN_GREATEREQUAL:    return ">=";
        case TOKEN_LESSEQUAL:       return "<=";
        case TOKEN_AND:             return "&&";
        case TOKEN_OR:              return "||";
        case TOKEN_NOT:             return "!";
        case TOKEN_QMARK:           return "?";
        case TOKEN_ASSIGN:          return "=";
        case TOKEN_PLUS_EQUAL:      return "+=";
        case TOKEN_MINUS_EQUAL:     return "-=";
        case TOKEN_MUL_EQUAL:       return "*=";
        case TOKEN_DIV_EQUAL:       return "/=";
        case TOKEN_MOD_EQUAL:       return "%=";
        case TOKEN_IDIV_EQUAL:      return "~=";
        case TOKEN_LPAREN:          return "(";
        case TOKEN_RPAREN:          return ")";
        case TOKEN_LBRACE:          return "{";
        case TOKEN_RBRACE:          return "}";
        case TOKEN_LBRACKET:        return "[";
        case TOKEN_RBRACKET:        return "]";
        case TOKEN_SEMICOLON:       return ";";
        case TOKEN_COMMA:           return ",";
        case TOKEN_COLON:           return ":";
        case TOKEN_DOT:             return ".";
        case TOKEN_EOF:             return "EOF";
        case INVALID:               return "INVALID";
        default:                    return "UNKNOWN_TOKEN";
    }
}