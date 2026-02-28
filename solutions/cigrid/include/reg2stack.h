#pragma once
#include "string.h"

#define MAX_REGS 100

// virtual register to stack offset mapping
typedef struct {
    char* reg;   // virtual register name
    int offset;  // stack offset
} RegToStack;

extern RegToStack reg_to_stack[MAX_REGS];
extern int reg_count;
extern int stack_offset;

// assign stack offset to virtual register
int get_stack_offset(const char* reg);
