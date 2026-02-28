#include "../include/ast.h"

// create AST node
ASTNode* create_node(const char* type, const char* value, int child_count, ...) {
    ASTNode* node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = strdup(type);
    node->value = value ? strdup(value) : NULL;
    node->child_count = child_count;
    node->children = (ASTNode**)malloc(sizeof(ASTNode*) * child_count);

    va_list args;
    va_start(args, child_count);
    for (int i = 0; i < child_count; i++) {
        node->children[i] = va_arg(args, ASTNode*);
    }
    va_end(args);

    return node;
}

// print AST
void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
     if (!strcmp(node->type,"bop")) {
        printf("%s",node->value);
    }

    if (!strcmp(node->type,"EInt")) {
        printf("%s",node->type);
        printf("(");
        printf("%s",node->value);
        printf(")");
    }
    if (!strcmp(node->type,"EChar")) {
        printf("%s",node->type);
        printf("(");
        printf("%s",node->value);
        printf(")");
    }
    if (!strcmp(node->type,"EVar")) {
        printf("%s",node->type);
        printf("(\"");
        printf("%s",node->value);
        printf("\")");
    }
    if (!strcmp(node->type,"T")) {
        if(!strcmp(node->value,"TVoid")){
            printf("%s",node->value);
        }
        else if (!strcmp(node->value,"TInt")){
            printf("%s",node->value);
        }
        else if (!strcmp(node->value,"TChar")){
            printf("%s",node->value);
        }

    }
    if (!strcmp(node->type,"l")) {
            if(!strcmp(node->value,"params_o")){
                printf("%s",node->value);
            }
            else if(!strcmp(node->value,"params_n")){   //ty T_IDENTIFIER
                printf("(");
                print_ast(node->children[0],0);
                printf(",\"");
                printf("%s",node->children[1]->value);
                printf("\")");
            }
            else if(!strcmp(node->value,"params_l")){   
                print_ast(node->children[0],0);
                print_ast(node->children[1],0);
            } 
    }
    
    if (!strcmp(node->type,"commaexpr")) {
        print_ast(node->children[0],0);
        printf(" ");
        print_ast(node->children[1],0);
    }

    if (!strcmp(node->type,"e")) {
        if (!strcmp(node->value,"EBinOp")){
            printf("%s",node->value);
            printf("(");
            print_ast(node->children[1],0);//bop
            printf(",");
            print_ast(node->children[0],0);//e
            printf(",");
            print_ast(node->children[2],0);//e
            printf(")");
        }
        if (!strcmp(node->value,"ECall")){
            printf("%s",node->value);
            printf("(\"");
            printf("%s",node->children[0]->value);//TextStr
            printf("\",");
            printf("{");
            print_ast(node->children[1],0);//e
            printf("}");
            printf(")");
        }
    }
    if (!strcmp(node->type,"s")) {
        if(!strcmp(node->value,"SScope")){
            printf("(");
            printf("{");
            print_ast(node -> children[0], 0);
            printf("}");
            printf(")");
        }
        if(!strcmp(node->value,"SReturn")){
            printf("(");
            if(node->children[0])
                print_ast(node -> children[0], 0);
            printf(")");   
        }
        if(!strcmp(node->value,"SExpr")){
            printf("(");
            if(node->children[0])
                print_ast(node ->children[0], 0);
            printf(")");   
        }
        if(!strcmp(node->value,"SIf")){
            printf("(");
            for (int i = 0; i < node->child_count; i++) {
                print_ast(node->children[i],0); // print each child statement
                if(i!=2) printf(",");
            }
            printf(")");   
        }
        if(!strcmp(node->value,"SVarAssign")){
            printf("(");
            if(node->children[0])
                print_ast(node ->children[0], 0);
            printf(")");   
        }
        if(!strcmp(node->value,"SVarDef")){
            printf("(");
            if(node->children[0])
                print_ast(node ->children[0], 0);
            printf(")");   
        }
        if(!strcmp(node->value,"SWhile")){
            printf("(");
            print_ast(node ->children[0], 0);
            printf(",");
            print_ast(node ->children[1], 0);
            printf(")");  
        }

        
    }
    if (!strcmp(node->type, "varassign")) {
            print_ast(node->children[0],0);
            printf(",");
            print_ast(node->children[1],0);

    }
    if (!strcmp(node->type, "tyassign")) {
            print_ast(node->children[0],0);
            printf(",");
            printf("\"%s\"",node->children[1]->value);
            printf(",");
            print_ast(node->children[2],0);

    }  
    if (!strcmp(node->type, "stmt_list")) {
        for (int i = 0; i < node->child_count; i++) {
            print_ast(node->children[i],0); // print each child statement
        }
    } 
    if (!strcmp(node->type, "lvalue")) {
        printf("\"%s\"",node->value);
    } 

    if(!strcmp(node->type,"g")){
        printf("%s", node->value);
        if(!strcmp(node->value,"GFuncDef")) {
            printf("(");
            printf("%s",node -> children[0] -> value);
            printf(",");
            printf("\"%s\"",node -> children[1] -> value);
            printf(",{");
            print_ast(node->children[2], 0);  // param list
            printf("},");
            print_ast(node->children[3], 0);  // function body
            printf(")\n");
        }
        if(!strcmp(node->value,"GFuncDecl")) {
            printf("(");
            printf("%s",node -> children[0] -> value);
            printf(",");
            printf("\"%s\"",node -> children[1] -> value);
            printf(",{");
            print_ast(node->children[2], 0);  // param list
            printf("})\n");
        }
    }
    if(!strcmp(node->type,"p")){
        for (int i = 0; i < node->child_count; i++) {
           print_ast(node->children[i],0); // print each child statement
        }
    }
}

// free AST
void free_ast(ASTNode* node) {
    if (!node) return;

    free(node->type);
    if (node->value) free(node->value);
    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    free(node->children);
    free(node);
}

void traverse_ast(const ASTNode* node) {
    if (node == NULL) {
        return;
    }

    printf("Node: Type='%s', Value='%s', Children=%d\n", node->type, node->value, node->child_count);

    // traverse children
    for (int i = 0; i < node->child_count; i++) {
        traverse_ast(node->children[i]);
    }
}

bool is_only_eint_and_binop(ASTNode* node, int* sum) {
    if (!node) return true;

    // if node is not EInt or BinOp, return false
    if (strcmp(node->type, "bop") != 0 &&
        strcmp(node->type, "e") != 0 &&
        strcmp(node->type, "EInt") != 0) {
        return false;
    }

    // if node is EInt, accumulate the value
    if (strcmp(node->type, "EInt") == 0) {
        *sum += atoi(node->value); // convert string to int
    }

    // check children
    for (int i = 0; i < node->child_count; i++) {
        if (!is_only_eint_and_binop(node->children[i], sum)) {
            return false;
        }
    }

    return true;
}
