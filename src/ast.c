#include "ast.h"
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
        
        case NODE_BLOCK: {
            ASTNode* stmt_list = (ASTNode*)root->specific_node;
            free_ast(stmt_list);
            break;
        }
        
        case NODE_CONTINUE_STMT:
        case NODE_STOP_STMT:
            // These nodes have no additional data
            free(root->specific_node);
            break;
        
        default:
            // For unhandled node types, just free the specific node if it exists
            if (root->specific_node) {
                free(root->specific_node);
            }
            break;
    }
    
    free(root);
}
