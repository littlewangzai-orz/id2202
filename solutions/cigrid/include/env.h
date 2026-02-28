#pragma once 

#include "ast.h"
#include "ASM_IR.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// things in the environment
typedef struct {
    char* sym;
    int id;
    ASTNode* ty;
} EnvEntry;

//environment, using dynamic arrays to manage the EnvEntry
typedef struct {
    EnvEntry* entries;
    int count;
    int capacity;
} Environment;

//return bitsize by the type
int bitsize_of_type(const char* type); 
//initialize the environment 
void init_env(Environment* env); 

//add_variable to the environment 
void add_variable(Environment *env, const char *name, int id, ASTNode* type); 

//find variable whether in the environment 
EnvEntry* find_variable(Environment *env, const char *sym);

//print the environment 
void print_environment(Environment *env);

//free environment memory
void free_env(Environment* env);

//make_reg
TReg make_reg(const char* l, Environment* env); 

//make temporary register
TReg tmp_reg(int n);
