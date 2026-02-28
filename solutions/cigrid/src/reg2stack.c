#include "../include/reg2stack.h"

RegToStack reg_to_stack[MAX_REGS];
int reg_count = 0;
int stack_offset = 0;

// store the stack offset for each register
int get_stack_offset(const char* reg) {
    // lookup wheter the register has been assigned a stack offset
    for (int i = 0; i < reg_count; i++) {
        if (strcmp(reg_to_stack[i].reg, reg) == 0) {
            return reg_to_stack[i].offset;
        }
    }
    // assign a new stack offset
    reg_to_stack[reg_count].reg = strdup(reg);
    reg_to_stack[reg_count].offset = stack_offset;
    stack_offset += 8;  // each register occupies 8 bytes
    
    return reg_to_stack[reg_count++].offset;
}
