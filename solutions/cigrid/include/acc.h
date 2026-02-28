#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ASM_IR.h"

typedef struct {
    Inst *instructions;  // dynamic array of instructions
    int count;                  // number of instructions
    int capacity;               // capacity of the array
} Accumulator;


void init_accumulator(Accumulator* acc);

void add_instruction(Accumulator* acc, Inst instruction);

void print_accumulator(const Accumulator* acc);

void free_accumulator(Accumulator* acc);

Inst replace_virtual_register(Inst inst); 

void register_spilling(Accumulator acc, int n);