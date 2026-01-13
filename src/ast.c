#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Node Creation
// ============================================================================

ASTNode* create_node(ASTNodeType type, void* specific_data) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for ASTNode\n");
        return NULL;
    }

    node->type = type;
    node->specific_node = specific_data;
    node->parent = NULL;
    node->line = 0;
    node->column = 0;

    return node;
}

// IMPROVEMENT: Helper to set location data immediately
ASTNode* create_node_with_loc(ASTNodeType type, void* specific_data, TokenData loc_token) {
    ASTNode* node = create_node(type, specific_data);
    if (node) {
        node->line = loc_token.loc.line;
        node->column = loc_token.loc.col;
    }
    return node;
}

// ============================================================================
// Memory Management - Recursive Cleanup
// ============================================================================

void free_ast(ASTNode* root) {
    if (!root) return;

    switch (root->type) {
        // Statement nodes with children
        case NODE_STATEMENT_LIST: {
            StatementListNode* list = (StatementListNode*)root->specific_node;
            if (list) {
                for (size_t i = 0; i < list->count; i++) {
                    free_ast(list->statements[i]);
                }
                free(list->statements);
                free(list);
            }
            break;
        }

        case NODE_BLOCK: {
            StatementListNode* block = (StatementListNode*)root->specific_node;
            if (block) {
                for (size_t i = 0; i < block->count; i++) {
                    free_ast(block->statements[i]);
                }
                free(block->statements);
                free(block);
            }
            break;
        }

        case NODE_PROGRAM: {
            ProgramNode* prog = (ProgramNode*)root->specific_node;
            if (prog) {
                for (size_t i = 0; i < prog->global_count; i++) {
                    free_ast(prog->global_decls[i]);
                }
                free(prog->global_decls);
                free_ast(prog->stmt_list);
                free(prog);
            }
            break;
        }

        // Declarations
        case NODE_VAR_DECL: {
            VarDeclNode* var = (VarDeclNode*)root->specific_node;
            if (var) {
                free(var->var_name);
                free(var->data_type);
                free_ast(var->init_expr);
                free(var);
            }
            break;
        }

        case NODE_FUNC_DECL: {
            FuncDeclNode* func = (FuncDeclNode*)root->specific_node;
            if (func) {
                free(func->return_type);
                free(func->func_name);
                free_ast(func->params);
                free_ast(func->body);
                free(func);
            }
            break;
        }

        case NODE_PARAM_LIST: {
            ParameterListNode* params = (ParameterListNode*)root->specific_node;
            if (params) {
                for (size_t i = 0; i < params->count; i++) {
                    free(params->parameters[i]->param_type);
                    free(params->parameters[i]->param_name);
                    free(params->parameters[i]);
                }
                free(params->parameters);
                free(params);
            }
            break;
        }

        // Statements
        case NODE_DECL_STMT: {
            DeclStmtNode* decl = (DeclStmtNode*)root->specific_node;
            if (decl) {
                free_ast(decl->decl);
                free(decl);
            }
            break;
        }

        case NODE_ASSIGN_STMT: {
            AssignStmtNode* assign = (AssignStmtNode*)root->specific_node;
            if (assign) {
                free(assign->var_name);
                free_ast(assign->expr);
                free(assign);
            }
            break;
        }

        case NODE_INPUT_STMT: {
            InputStmtNode* input = (InputStmtNode*)root->specific_node;
            if (input) {
                free(input->var_name);
                free(input->data_type);
                free(input);
            }
            break;
        }

        case NODE_OUTPUT_STMT: {
            OutputStmtNode* output = (OutputStmtNode*)root->specific_node;
            if (output) {
                free_ast(output->expr);
                free(output);
            }
            break;
        }

        case NODE_COND_STMT: {
            CondStmtNode* cond = (CondStmtNode*)root->specific_node;
            if (cond) {
                free_ast(cond->condition);
                free_ast(cond->then_body);
                free_ast(cond->else_body);
                free(cond);
            }
            break;
        }

        case NODE_ITER_STMT: {
            IterStmtNode* iter = (IterStmtNode*)root->specific_node;
            if (iter) {
                free_ast(iter->init);
                free_ast(iter->condition);
                free_ast(iter->increment);
                free_ast(iter->body);
                free(iter);
            }
            break;
        }

        case NODE_RETURN_STMT: {
            ReturnStmtNode* ret = (ReturnStmtNode*)root->specific_node;
            if (ret) {
                free_ast(ret->expr);
                free(ret);
            }
            break;
        }

        case NODE_FUNC_CALL: {
            FuncCallNode* call = (FuncCallNode*)root->specific_node;
            if (call) {
                free(call->func_name);
                for (size_t i = 0; i < call->count; i++) {
                    free_ast(call->arguments[i]);
                }
                free(call->arguments);
                free(call);
            }
            break;
        }

        // Expression nodes
        case NODE_BINARY_EXPR: {
            BinaryExpressionNode* binop = (BinaryExpressionNode*)root->specific_node;
            if (binop) {
                free_ast(binop->left);
                free_ast(binop->right);
                free(binop);
            }
            break;
        }

        case NODE_UNARY_EXPR: {
            UnaryExpressionNode* unop = (UnaryExpressionNode*)root->specific_node;
            if (unop) {
                free_ast(unop->operand);
                free(unop);
            }
            break;
        }

        case NODE_IDENTIFIER: {
            IdentifierNode* id = (IdentifierNode*)root->specific_node;
            if (id) {
                free(id->name);
                free(id);
            }
            break;
        }

        case NODE_LITERAL: {
            LiteralNode* lit = (LiteralNode*)root->specific_node;
            if (lit) {
                if (lit->literal_type == LITERAL_STRING) {
                    free(lit->value.string_value);
                }
                free(lit);
            }
            break;
        }

        case NODE_CONTINUE_STMT:
        case NODE_STOP_STMT:
            // These have no specific_node data
            break;

        case NODE_ARG_LIST: {
            ArgListNode* args = (ArgListNode*)root->specific_node;
            if (args) {
                for (size_t i = 0; i < args->count; i++) {
                    free_ast(args->args[i]);
                }
                free(args->args);
                free(args);
            }
            break;
        }

        default:
            break;
    }

    free(root);
}

// ============================================================================
// Statement List Management (Improved Safety)
// ============================================================================

void add_statement(StatementListNode* list, ASTNode* stmt) {
    if (!list || !stmt) return;

    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 10 : list->capacity * 2;
        // IMPROVEMENT: Use temporary pointer for realloc safety
        ASTNode** temp = (ASTNode**)realloc(list->statements, new_capacity * sizeof(ASTNode*));
        if (!temp) {
            fprintf(stderr, "Fatal Error: Memory reallocation failed for statements\n");
            // In a real compiler, we might exit or longjmp here. 
            // Returning leaves the list intact but incomplete.
            return;
        }
        list->statements = temp;
        list->capacity = new_capacity;
    }

    list->statements[list->count++] = stmt;
    stmt->parent = (ASTNode*)list;
}

// ============================================================================
// Program Management (Improved Safety)
// ============================================================================

void add_global_decl(ProgramNode* program, ASTNode* decl) {
    if (!program || !decl) return;

    if (program->global_count >= program->global_capacity) {
        size_t new_capacity = (program->global_capacity == 0) ? 10 : program->global_capacity * 2;
        // IMPROVEMENT: Use temporary pointer for realloc safety
        ASTNode** temp = (ASTNode**)realloc(program->global_decls, new_capacity * sizeof(ASTNode*));
        if (!temp) {
            fprintf(stderr, "Fatal Error: Memory reallocation failed for globals\n");
            return;
        }
        program->global_decls = temp;
        program->global_capacity = new_capacity;
    }

    program->global_decls[program->global_count++] = decl;
    decl->parent = NULL;  // Global scope
}

// ============================================================================
// Parameter List Management (Improved Safety)
// ============================================================================

void add_parameter(ParameterListNode* list, ParameterNode* param) {
    if (!list || !param) return;

    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 5 : list->capacity * 2;
        // IMPROVEMENT: Use temporary pointer for realloc safety
        ParameterNode** temp = (ParameterNode**)realloc(list->parameters, new_capacity * sizeof(ParameterNode*));
        if (!temp) {
            fprintf(stderr, "Fatal Error: Memory reallocation failed for parameters\n");
            return;
        }
        list->parameters = temp;
        list->capacity = new_capacity;
    }

    list->parameters[list->count++] = param;
}

// ============================================================================
// Function Call Argument Management (Improved Safety)
// ============================================================================

void add_argument(FuncCallNode* call, ASTNode* arg) {
    if (!call || !arg) return;

    if (call->count >= call->capacity) {
        size_t new_capacity = (call->capacity == 0) ? 5 : call->capacity * 2;
        // IMPROVEMENT: Use temporary pointer for realloc safety
        ASTNode** temp = (ASTNode**)realloc(call->arguments, new_capacity * sizeof(ASTNode*));
        if (!temp) {
            fprintf(stderr, "Fatal Error: Memory reallocation failed for arguments\n");
            return;
        }
        call->arguments = temp;
        call->capacity = new_capacity;
    }

    call->arguments[call->count++] = arg;
    arg->parent = (ASTNode*)call;
}

// ============================================================================
// AST Pretty Printing
// ============================================================================

static const char* get_node_type_name(ASTNodeType type) {
    switch (type) {
        case NODE_PROGRAM:           return "PROGRAM";
        case NODE_STATEMENT_LIST:    return "STATEMENT_LIST";
        case NODE_PARAM_LIST:        return "PARAM_LIST";
        case NODE_ARG_LIST:          return "ARG_LIST";
        case NODE_DECL_STMT:         return "DECL_STMT";
        case NODE_VAR_DECL:          return "VAR_DECL";
        case NODE_FUNC_DECL:         return "FUNC_DECL";
        case NODE_ASSIGN_STMT:       return "ASSIGN_STMT";
        case NODE_INPUT_STMT:        return "INPUT_STMT";
        case NODE_OUTPUT_STMT:       return "OUTPUT_STMT";
        case NODE_COND_STMT:         return "COND_STMT";
        case NODE_ITER_STMT:         return "ITER_STMT";
        case NODE_RETURN_STMT:       return "RETURN_STMT";
        case NODE_CONTINUE_STMT:     return "CONTINUE_STMT";
        case NODE_STOP_STMT:         return "STOP_STMT";
        case NODE_BLOCK:             return "BLOCK";
        case NODE_FUNC_CALL:         return "FUNC_CALL";
        case NODE_BINARY_EXPR:       return "BINARY_EXPR";
        case NODE_UNARY_EXPR:        return "UNARY_EXPR";
        case NODE_LITERAL:           return "LITERAL";
        case NODE_IDENTIFIER:        return "IDENTIFIER";
        default:                     return "UNKNOWN";
    }
}

static const char* get_binary_op_name(int op) {
    switch (op) {
        case OP_ADD:      return "+";
        case OP_SUB:      return "-";
        case OP_MUL:      return "*";
        case OP_DIV:      return "/";
        case OP_MOD:      return "%";
        case OP_IDIV:     return "~";
        case OP_POW:      return "^";
        case OP_EQ:       return "==";
        case OP_NEQ:      return "!=";
        case OP_LT:       return "<";
        case OP_GT:       return ">";
        case OP_LTE:      return "<=";
        case OP_GTE:      return ">=";
        case OP_AND:      return "&&";
        case OP_OR:       return "||";
        default:          return "???";
    }
}

static const char* get_unary_op_name(int op) {
    switch (op) {
        case UNOP_NEG:    return "-";
        case UNOP_NOT:    return "!";
        case UNOP_POS:    return "+";
        default:          return "???";
    }
}

static const char* get_assign_type_name(int type) {
    switch (type) {
        case ASSIGN_DIRECT:  return "=";
        case ASSIGN_PLUS:    return "+=";
        case ASSIGN_MINUS:   return "-=";
        case ASSIGN_MUL:     return "*=";
        case ASSIGN_DIV:     return "/=";
        case ASSIGN_MOD:     return "%=";
        default:             return "???";
    }
}

static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    print_indent(indent);
    printf("NODE_%s", get_node_type_name(node->type));

    // Print node-specific metadata
    if (node->specific_node) {
        switch (node->type) {
            case NODE_IDENTIFIER: {
                IdentifierNode* id = (IdentifierNode*)node->specific_node;
                printf(" (name: \"%s\")", id->name);
                break;
            }

            case NODE_LITERAL: {
                LiteralNode* lit = (LiteralNode*)node->specific_node;
                switch (lit->literal_type) {
                    case LITERAL_NUMBER:
                        printf(" (value: %d)", lit->value.int_value);
                        break;
                    case LITERAL_DECIMAL:
                        printf(" (value: %f)", lit->value.double_value);
                        break;
                    case LITERAL_STRING:
                        printf(" (value: \"%s\")", lit->value.string_value);
                        break;
                    case LITERAL_CHAR:
                        printf(" (value: '%c')", lit->value.char_value);
                        break;
                    case LITERAL_BOOL:
                        printf(" (value: %s)", lit->value.bool_value ? "true" : "false");
                        break;
                    case LITERAL_NULL:
                        printf(" (value: null)");
                        break;
                }
                break;
            }

            case NODE_BINARY_EXPR: {
                BinaryExpressionNode* binop = (BinaryExpressionNode*)node->specific_node;
                printf(" (op: %s)", get_binary_op_name(binop->operator));
                break;
            }

            case NODE_UNARY_EXPR: {
                UnaryExpressionNode* unop = (UnaryExpressionNode*)node->specific_node;
                printf(" (op: %s)", get_unary_op_name(unop->operator));
                break;
            }

            case NODE_VAR_DECL: {
                VarDeclNode* var = (VarDeclNode*)node->specific_node;
                printf(" (name: \"%s\", type: \"%s\", const: %d)",
                       var->var_name, var->data_type, var->is_const);
                break;
            }

            case NODE_FUNC_DECL: {
                FuncDeclNode* func = (FuncDeclNode*)node->specific_node;
                printf(" (name: \"%s\", return_type: \"%s\")",
                       func->func_name, func->return_type);
                break;
            }

            case NODE_ASSIGN_STMT: {
                AssignStmtNode* assign = (AssignStmtNode*)node->specific_node;
                printf(" (var: \"%s\", op: %s)",
                       assign->var_name, get_assign_type_name(assign->assign_type));
                break;
            }

            case NODE_INPUT_STMT: {
                InputStmtNode* input = (InputStmtNode*)node->specific_node;
                printf(" (var: \"%s\", type: \"%s\")",
                       input->var_name, input->data_type);
                break;
            }

            case NODE_FUNC_CALL: {
                FuncCallNode* call = (FuncCallNode*)node->specific_node;
                printf(" (func: \"%s\", args: %zu)",
                       call->func_name, call->count);
                break;
            }

            default:
                break;
        }
    }

    printf(" (line: %zu, col: %zu)\n", node->line, node->column);

    // Print children recursively
    switch (node->type) {
        case NODE_STATEMENT_LIST:
        case NODE_BLOCK: {
            StatementListNode* list = (StatementListNode*)node->specific_node;
            if (list) {
                for (size_t i = 0; i < list->count; i++) {
                    print_ast(list->statements[i], indent + 1);
                }
            }
            break;
        }

        case NODE_PROGRAM: {
            ProgramNode* prog = (ProgramNode*)node->specific_node;
            if (prog) {
                if (prog->global_count > 0) {
                    print_indent(indent + 1);
                    printf("NODE_GLOBALS\n");
                    for (size_t i = 0; i < prog->global_count; i++) {
                        print_ast(prog->global_decls[i], indent + 2);
                    }
                }
                if (prog->stmt_list) {
                    print_indent(indent + 1);
                    printf("NODE_MAIN\n");
                    print_ast(prog->stmt_list, indent + 2);
                }
            }
            break;
        }

        case NODE_PARAM_LIST: {
            ParameterListNode* params = (ParameterListNode*)node->specific_node;
            if (params) {
                for (size_t i = 0; i < params->count; i++) {
                    print_indent(indent + 1);
                    printf("PARAM (type: \"%s\", name: \"%s\")\n",
                           params->parameters[i]->param_type,
                           params->parameters[i]->param_name);
                }
            }
            break;
        }

        case NODE_VAR_DECL: {
            VarDeclNode* var = (VarDeclNode*)node->specific_node;
            if (var && var->init_expr) {
                print_indent(indent + 1);
                printf("INIT_EXPR\n");
                print_ast(var->init_expr, indent + 2);
            }
            break;
        }

        case NODE_FUNC_DECL: {
            FuncDeclNode* func = (FuncDeclNode*)node->specific_node;
            if (func) {
                if (func->params) {
                    print_ast(func->params, indent + 1);
                }
                if (func->body) {
                    print_ast(func->body, indent + 1);
                }
            }
            break;
        }

        case NODE_ASSIGN_STMT: {
            AssignStmtNode* assign = (AssignStmtNode*)node->specific_node;
            if (assign && assign->expr) {
                print_ast(assign->expr, indent + 1);
            }
            break;
        }

        case NODE_OUTPUT_STMT: {
            OutputStmtNode* output = (OutputStmtNode*)node->specific_node;
            if (output && output->expr) {
                print_ast(output->expr, indent + 1);
            }
            break;
        }

        case NODE_COND_STMT: {
            CondStmtNode* cond = (CondStmtNode*)node->specific_node;
            if (cond) {
                print_indent(indent + 1);
                printf("CONDITION\n");
                print_ast(cond->condition, indent + 2);
                print_indent(indent + 1);
                printf("THEN_BODY\n");
                print_ast(cond->then_body, indent + 2);
                if (cond->else_body) {
                    print_indent(indent + 1);
                    printf("ELSE_BODY\n");
                    print_ast(cond->else_body, indent + 2);
                }
            }
            break;
        }

        case NODE_ITER_STMT: {
            IterStmtNode* iter = (IterStmtNode*)node->specific_node;
            if (iter) {
                print_indent(indent + 1);
                printf("INIT\n");
                print_ast(iter->init, indent + 2);
                print_indent(indent + 1);
                printf("CONDITION\n");
                print_ast(iter->condition, indent + 2);
                print_indent(indent + 1);
                printf("INCREMENT\n");
                print_ast(iter->increment, indent + 2);
                print_indent(indent + 1);
                printf("BODY\n");
                print_ast(iter->body, indent + 2);
            }
            break;
        }

        case NODE_RETURN_STMT: {
            ReturnStmtNode* ret = (ReturnStmtNode*)node->specific_node;
            if (ret && ret->expr) {
                print_ast(ret->expr, indent + 1);
            }
            break;
        }

        case NODE_BINARY_EXPR: {
            BinaryExpressionNode* binop = (BinaryExpressionNode*)node->specific_node;
            if (binop) {
                print_ast(binop->left, indent + 1);
                print_ast(binop->right, indent + 1);
            }
            break;
        }

        case NODE_UNARY_EXPR: {
            UnaryExpressionNode* unop = (UnaryExpressionNode*)node->specific_node;
            if (unop && unop->operand) {
                print_ast(unop->operand, indent + 1);
            }
            break;
        }

        case NODE_FUNC_CALL: {
            FuncCallNode* call = (FuncCallNode*)node->specific_node;
            if (call) {
                for (size_t i = 0; i < call->count; i++) {
                    print_ast(call->arguments[i], indent + 1);
                }
            }
            break;
        }

        case NODE_ARG_LIST: {
            ArgListNode* args = (ArgListNode*)node->specific_node;
            if (args) {
                for (size_t i = 0; i < args->count; i++) {
                    print_ast(args->args[i], indent + 1);
                }
            }
            break;
        }

        case NODE_DECL_STMT: {
            DeclStmtNode* decl = (DeclStmtNode*)node->specific_node;
            if (decl && decl->decl) {
                print_ast(decl->decl, indent + 1);
            }
            break;
        }

        default:
            break;
    }
}