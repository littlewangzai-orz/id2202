#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

// AST Node 
typedef struct ASTNode {
    char *type;               // Node Type£¬eg.stmt
    char *value;              // Node£¬eg."GVarDef", "SIf"
    struct ASTNode **children; // child nodes
    int child_count;          // child node count
} ASTNode;


ASTNode* create_node(const char* type, const char* value, int child_count, ...);

void print_ast(ASTNode* node, int indent);

void free_ast(ASTNode* node);

void traverse_ast(const ASTNode* node);

bool is_only_eint_and_binop(ASTNode* node, int* sum); 

void traverse_ast(const ASTNode* node); 