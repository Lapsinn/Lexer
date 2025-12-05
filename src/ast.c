#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Factory function to create a generic AST node
ASTNode* create_node(ASTNodeType type, void* specific_data) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    
    node->type = type;
    node->parent = NULL;
    node->specific_node = specific_data;
    node->line = 0;
    node->column = 0;
    
    return node;
}

// Helper function to add statement to list
void add_statement(StatementListNode* list, ASTNode* stmt) {
    if (!list) return;
    
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        ASTNode** new_stmts = (ASTNode**)realloc(list->statements, 
                                                  new_capacity * sizeof(ASTNode*));
        if (!new_stmts) return;
        
        list->statements = new_stmts;
        list->capacity = new_capacity;
    }
    
    list->statements[list->count++] = stmt;
}

// Recursive function to free AST nodes
void free_ast(ASTNode* root) {
    if (!root) return;
    
    switch (root->type) {
        case NODE_PROGRAM: {
            ASTNode* stmt_list = (ASTNode*)root->specific_node;
            free_ast(stmt_list);
            break;
        }
        
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
        
        case NODE_BINARY_EXPR: {
            BinaryExpressionNode* bin = (BinaryExpressionNode*)root->specific_node;
            if (bin) {
                free_ast(bin->left);
                free_ast(bin->right);
                free(bin);
            }
            break;
        }
        
        case NODE_UNARY_EXPR: {
            UnaryExpressionNode* unary = (UnaryExpressionNode*)root->specific_node;
            if (unary) {
                free_ast(unary->operand);
                free(unary);
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
                if (lit->literal_type == LITERAL_STRING && lit->value.string_value) {
                    free(lit->value.string_value);
                }
                free(lit);
            }
            break;
        }
        
        case NODE_VAR_DECL: {
            VarDeclNode* decl = (VarDeclNode*)root->specific_node;
            if (decl) {
                free(decl->var_name);
                free(decl->data_type);
                free_ast(decl->init_expr);
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
        
        case NODE_OUTPUT_STMT: {
            OutputStmtNode* output = (OutputStmtNode*)root->specific_node;
            if (output) {
                free_ast(output->expr);
                free(output);
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
        
        case NODE_CONTINUE_STMT:
        case NODE_STOP_STMT:
            // These nodes have no additional data
            free(root->specific_node);
            break;
        
        default:
            if (root->specific_node) {
                free(root->specific_node);
            }
            break;
    }
    
    free(root);
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static const char* get_operator_name(int op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_IDIV: return "~";
        case OP_POW: return "^";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LTE: return "<=";
        case OP_GTE: return ">=";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        default: return "?";
    }
}

static const char* get_unary_operator_name(int op) {
    switch (op) {
        case UNOP_NEG: return "-";
        case UNOP_NOT: return "!";
        case UNOP_POS: return "+";
        default: return "?";
    }
}

static const char* get_assign_type_name(int type) {
    switch (type) {
        case ASSIGN_DIRECT: return "=";
        case ASSIGN_PLUS: return "+=";
        case ASSIGN_MINUS: return "-=";
        case ASSIGN_MUL: return "*=";
        case ASSIGN_DIV: return "/=";
        case ASSIGN_MOD: return "%=";
        default: return "?";
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }
    
    print_indent(indent);
    
    switch (node->type) {
        case NODE_PROGRAM: {
            printf("PROGRAM\n");
            ASTNode* stmt_list = (ASTNode*)node->specific_node;
            print_ast(stmt_list, indent + 1);
            break;
        }
        
        case NODE_STATEMENT_LIST: {
            StatementListNode* list = (StatementListNode*)node->specific_node;
            printf("STATEMENT_LIST (%zu statements)\n", list ? list->count : 0);
            if (list) {
                for (size_t i = 0; i < list->count; i++) {
                    print_ast(list->statements[i], indent + 1);
                }
            }
            break;
        }
        
        case NODE_VAR_DECL: {
            VarDeclNode* decl = (VarDeclNode*)node->specific_node;
            printf("VAR_DECL: %s%s %s\n", 
                   decl->is_const ? "const " : "",
                   decl->data_type,
                   decl->var_name);
            if (decl->init_expr) {
                print_indent(indent + 1);
                printf("Initializer:\n");
                print_ast(decl->init_expr, indent + 2);
            }
            break;
        }
        
        case NODE_ASSIGN_STMT: {
            AssignStmtNode* assign = (AssignStmtNode*)node->specific_node;
            printf("ASSIGN: %s %s\n", 
                   assign->var_name,
                   get_assign_type_name(assign->assign_type));
            print_ast(assign->expr, indent + 1);
            break;
        }
        
        case NODE_OUTPUT_STMT: {
            OutputStmtNode* output = (OutputStmtNode*)node->specific_node;
            printf("OUTPUT (show)\n");
            print_ast(output->expr, indent + 1);
            break;
        }
        
        case NODE_INPUT_STMT: {
            InputStmtNode* input = (InputStmtNode*)node->specific_node;
            printf("INPUT (ask): %s = ask(%s)\n", 
                   input->var_name,
                   input->data_type);
            break;
        }
        
        case NODE_COND_STMT: {
            CondStmtNode* cond = (CondStmtNode*)node->specific_node;
            printf("IF\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(cond->condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            print_ast(cond->then_body, indent + 2);
            if (cond->else_body) {
                print_indent(indent + 1);
                printf("Else:\n");
                print_ast(cond->else_body, indent + 2);
            }
            break;
        }
        
        case NODE_ITER_STMT: {
            IterStmtNode* iter = (IterStmtNode*)node->specific_node;
            printf("REPEAT\n");
            print_indent(indent + 1);
            printf("Init:\n");
            print_ast(iter->init, indent + 2);
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(iter->condition, indent + 2);
            print_indent(indent + 1);
            printf("Increment:\n");
            print_ast(iter->increment, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(iter->body, indent + 2);
            break;
        }
        
        case NODE_RETURN_STMT: {
            ReturnStmtNode* ret = (ReturnStmtNode*)node->specific_node;
            printf("RETURN\n");
            if (ret->expr) {
                print_ast(ret->expr, indent + 1);
            }
            break;
        }
        
        case NODE_CONTINUE_STMT: {
            printf("CONTINUE\n");
            break;
        }
        
        case NODE_STOP_STMT: {
            printf("STOP\n");
            break;
        }
        
        case NODE_BLOCK: {
            printf("BLOCK\n");
            ASTNode* stmt_list = (ASTNode*)node->specific_node;
            print_ast(stmt_list, indent + 1);
            break;
        }
        
        case NODE_BINARY_EXPR: {
            BinaryExpressionNode* bin = (BinaryExpressionNode*)node->specific_node;
            printf("BINARY_EXPR (%s)\n", get_operator_name(bin->operator));
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(bin->left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(bin->right, indent + 2);
            break;
        }
        
        case NODE_UNARY_EXPR: {
            UnaryExpressionNode* unary = (UnaryExpressionNode*)node->specific_node;
            printf("UNARY_EXPR (%s)\n", get_unary_operator_name(unary->operator));
            print_ast(unary->operand, indent + 1);
            break;
        }
        
        case NODE_IDENTIFIER: {
            IdentifierNode* id = (IdentifierNode*)node->specific_node;
            printf("IDENTIFIER: %s\n", id->name);
            break;
        }
        
        case NODE_LITERAL: {
            LiteralNode* lit = (LiteralNode*)node->specific_node;
            printf("LITERAL: ");
            switch (lit->literal_type) {
                case LITERAL_NUMBER:
                    printf("%d (number)\n", lit->value.int_value);
                    break;
                case LITERAL_DECIMAL:
                    printf("%f (decimal)\n", lit->value.double_value);
                    break;
                case LITERAL_STRING:
                    printf("\"%s\" (string)\n", lit->value.string_value);
                    break;
                case LITERAL_CHAR:
                    printf("'%c' (char)\n", lit->value.char_value);
                    break;
                case LITERAL_BOOL:
                    printf("%s (bool)\n", lit->value.bool_value ? "true" : "false");
                    break;
                case LITERAL_NULL:
                    printf("null\n");
                    break;
            }
            break;
        }
        
        default:
            printf("UNKNOWN_NODE_TYPE (%d)\n", node->type);
            break;
    }
}