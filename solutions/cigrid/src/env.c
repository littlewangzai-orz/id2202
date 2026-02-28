#include "../include/env.h"
#include "../include/ast.h"
#include "../include/ASM_IR.h"

void init_env(Environment* env) {
    env->count = 0;
    env->capacity = 10; // initial capacity
    env->entries = malloc(env->capacity * sizeof(EnvEntry));
    if (!env->entries) {
        fprintf(stderr, "Failed to allocate memory for environment\n");
        exit(1);
    }
}

void add_variable(Environment *env, const char *sym, int id, ASTNode* ty) {
    // double the capacity if the environment is full
    if (env->count >= env->capacity) {
        env->capacity *= 2;
        env->entries = (EnvEntry *)realloc(env->entries, sizeof(EnvEntry) * env->capacity);
    }

    // create a new entry
    env->entries[env->count].sym = strdup(sym); // copy the symbol
    env->entries[env->count].id = id;           // set the ID
    env->entries[env->count].ty = ty;           // set the type
    env->count++;
}


EnvEntry* find_variable(Environment *env, const char *sym) {
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].sym, sym) == 0) {
            return &env->entries[i];  // return the entry
        }
    }
    return NULL;  // not found
}

void print_environment(Environment *env) {
    printf("Environment:\n");
    
    for (int i = 0; i < env->count; i++) {
        printf("Variable %s -> ID: %d, Type: %s\n",
               env->entries[i].sym,
               env->entries[i].id,
               env->entries[i].ty->value);  // print the symbol, ID, and type
    }
}

void free_env(Environment* env){
    for (int i = 0; i < env->count; i++) {
        free(env->entries[i].sym);
        free_ast(env->entries[i].ty);
    }
    free(env->entries);
}


int bitsize_of_type(const char* type) {
    if (strcmp(type, "TInt") == 0) {
        return 32;
    } else if (strcmp(type, "TChar") == 0) {
        return 8;
    } 
    else return 0;
}

TReg make_reg(const char* l, Environment* env) {
    EnvEntry* entry = find_variable(env, l);
    if (entry == NULL) {
        fprintf(stderr, "Error: Variable %s not found in environment\n", l);
        exit(EXIT_FAILURE);
    }

    TReg treg;
    treg.reg.reg_number = entry->id;
    treg.reg.size= bitsize_of_type(entry->ty->value);
    treg.sym = malloc(32 * sizeof(char)); //malloc mem
    sprintf(treg.sym, "%s_%d", "x", entry->id);// gerenate the register name

    return treg;
}

TReg tmp_reg(int n) {
    TReg treg;
    treg.reg.reg_number = n;
    treg.reg.size = QWORD;
    
    treg.sym = malloc(64 * sizeof(char));  

    sprintf(treg.sym, "tmp_%d", n);  //gerenate the register name
    return treg;
}
