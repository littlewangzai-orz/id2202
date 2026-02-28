#pragma once
#include "../include/ASM_IR.h"
#include <string.h>


Operand make_imm_operand(int value) {
    Operand op;
    op.type = OP_IMM;
    op.imm = value;
    return op;
}

Operand make_reg_operand(const char* reg_name) {
    Operand op;
    op.type = OP_REG;
    op.treg.sym = strdup(reg_name);
    return op;
}

Operand make_treg_operand(TReg reg) {
    Operand op;
    op.type = OP_TREG;
    op.treg = reg;
    return op;
}

Inst create_binop_instruction(BinOp op_type, Operand dest, Operand src) {
    Inst inst;
    inst.type = INST_BINOP;
    inst.binop.op = op_type;
    inst.binop.dest = dest;
    inst.binop.src = src;
    return inst;
}

Inst create_ret_instruction() {
    // instruction for return
    Inst inst;
    inst.type = INST_RET;
    return inst;
}