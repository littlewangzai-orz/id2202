#include "../include/acc.h"
#include "../include/ASM_IR.h"
#include "../include/reg2stack.h"

// Initialize the accumulator
void init_accumulator(Accumulator* acc) {
    acc->count = 0;
    acc->capacity = 10;
    acc->instructions = malloc(acc->capacity * sizeof(Inst));
    if (!acc->instructions) {
        fprintf(stderr, "Failed to allocate memory for instructions\n");
        exit(1);
    }
}

void add_instruction(Accumulator* acc, Inst instruction) {
    if (acc->count >= acc->capacity) {
        acc->capacity *= 2;
        acc->instructions = realloc(acc->instructions, acc->capacity * sizeof(Inst));
        if (!acc->instructions) {
            fprintf(stderr, "Failed to reallocate memory for instructions\n");
            exit(1);
        }
    }
    acc->instructions[acc->count] = instruction;
    acc->count++;
}

void print_accumulator(const Accumulator* acc) {
    if (acc->count == 0) {
        printf("No instructions in accumulator\n");
    }
    else {
            for (int i = 0; i < acc->count; i++) {
            Inst inst = acc->instructions[i];
            switch (inst.type) {
             case INST_UNOP:
             case INST_BINOP:{
                    if(inst.binop.op == BINOP_MOV){
                        if(inst.binop.src.type == OP_IMM){
                            printf("        mov     %s, %d\n", inst.binop.dest.treg.sym, inst.binop.src.imm);
                            break;
                        }
                        else if(inst.binop.src.type == OP_TREG){
                            printf("        mov     %s, %s\n", inst.binop.dest.treg.sym, inst.binop.src.treg.sym);
                            //printf("        address     %p, %p\n", (void*)&inst.binop.dest.treg.sym, (void*)&inst.binop.src.treg.sym);
                            break;
                        }
                    }
                    else if(inst.binop.op == BINOP_ADD){
                        if(inst.binop.src.type != OP_IMM){
                            printf("        add     %s, %s\n", inst.binop.dest.treg.sym, inst.binop.src.treg.sym);
                        }
                        else if(inst.binop.src.type == OP_IMM){
                            printf("        add     %s, %d\n", inst.binop.dest.treg.sym, inst.binop.src.imm);
                        }
                        break;
                    }
                    else if(inst.binop.op == BINOP_SUB){
                        printf("        SUB (dest: %s, src: %s)\n", inst.binop.dest.treg.sym, inst.binop.src.treg.sym);
                        break;
                    }
                   
                }  
            case INST_CALL:
            case INST_CQO:
            case INST_RET:{
                    printf("        ret\n");
                    break;
                }
            default:
                 printf("instruction type:%d not supported\n", inst.type);
            }
        }
    }
}

void free_accumulator(Accumulator* acc) {
    free(acc->instructions);
    acc->instructions = NULL;
    acc->count = 0;
    acc->capacity = 0;
}


Inst replace_virtual_register(Inst inst) {

    Inst new_inst = inst;
    char buffer[64];
    // replace virtual register with stack address
    
    if (strncmp(inst.binop.dest.treg.sym, "x_", 2) == 0) {

        new_inst.binop.dest.type = inst.binop.dest.type;

        new_inst.binop.op = inst.binop.op;
        if(!get_stack_offset(inst.binop.dest.treg.sym))
            sprintf(buffer, "qword [rsp]");
        else
            sprintf(buffer, "qword [rsp + %d]", get_stack_offset(inst.binop.dest.treg.sym));
        free(new_inst.binop.dest.treg.sym);  // free old sym
        new_inst.binop.dest.treg.sym = strdup(buffer);  // assign new sym

        
        new_inst.binop.dest.treg.reg = inst.binop.dest.treg.reg;

        new_inst.binop.src = inst.binop.src;
    }
    else if (strncmp(inst.binop.dest.treg.sym, "tmp_", 4) == 0) {

        new_inst.binop.dest.type = inst.binop.dest.type;

        new_inst.binop.op = inst.binop.op;
        if(!get_stack_offset(inst.binop.dest.treg.sym))
            sprintf(buffer, "qword [rsp]");
        else
            sprintf(buffer, "qword [rsp + %d]", get_stack_offset(inst.binop.dest.treg.sym));

        new_inst.binop.dest.treg.sym = strdup(buffer);  // assign new sym
        new_inst.binop.dest.treg.reg = inst.binop.dest.treg.reg;

        new_inst.binop.src = inst.binop.src;

        
    }


    return new_inst;
}

void register_spilling(Accumulator acc, int n) {

  if(n) printf("        sub     rsp, %d\n", n * 8);
  for (int i = 0; i < acc.count; i++) {
    Inst inst = acc.instructions[i];
    // one instruction by one instruction
    if (inst.type == INST_BINOP) {

      if (inst.binop.dest.type == OP_TREG && inst.binop.src.type == OP_IMM) {
        Inst new_inst = replace_virtual_register(inst);
        printf("        mov     %s, %d\n", new_inst.binop.dest.treg.sym,
               new_inst.binop.src.imm);
        if(i == acc.count - 2){
            printf("        add     rsp, %d\n", n * 8);
        }
      }
      if (inst.binop.dest.type == OP_TREG && inst.binop.src.type == OP_TREG) {
        if (inst.binop.op == BINOP_MOV) {
          char buffer[64];
          if (!get_stack_offset(inst.binop.src.treg.sym))
            sprintf(buffer, "qword [rsp]");
          else
            sprintf(buffer, "qword [rsp + %d]",
                    get_stack_offset(inst.binop.src.treg.sym));

          printf("        mov     %s, %s\n", "r10", buffer);

          if (!get_stack_offset(inst.binop.dest.treg.sym))
            sprintf(buffer, "qword [rsp]");
          else
            sprintf(buffer, "qword [rsp + %d]",
                    get_stack_offset(inst.binop.dest.treg.sym));
          printf("        mov     %s, %s\n", buffer, "r10");
          if(i == acc.count - 2){
            printf("        add     rsp, %d\n", n * 8);
        }
        } else if (inst.binop.op == BINOP_ADD) {
          char buffer[64];
          if (!get_stack_offset(inst.binop.src.treg.sym))
            sprintf(buffer, "qword [rsp]");
          else
            sprintf(buffer, "qword [rsp + %d]",
                    get_stack_offset(inst.binop.src.treg.sym));
          printf("        mov     %s, %s\n", "r10", buffer);

          if (!get_stack_offset(inst.binop.dest.treg.sym))
            sprintf(buffer, "qword [rsp]");
          else
            sprintf(buffer, "qword [rsp + %d]",
                    get_stack_offset(inst.binop.dest.treg.sym));
          printf("        add     %s, %s\n", buffer, "r10");
          if(i == acc.count - 2){
            printf("        add     rsp, %d\n", n * 8);
        }
        }
      }
      if (inst.binop.dest.type == OP_REG && inst.binop.src.type == OP_TREG) {
        if(inst.binop.op == BINOP_MOV){
            char buffer[64];
            if (!get_stack_offset(inst.binop.src.treg.sym))
                sprintf(buffer, "qword [rsp]");
            else
                sprintf(buffer, "qword [rsp + %d]",
                  get_stack_offset(inst.binop.src.treg.sym));
            printf("        mov     %s, %s\n", inst.binop.dest.treg.sym, buffer);
        }
        else if(inst.binop.op == BINOP_ADD){
            char buffer[64];
            if (!get_stack_offset(inst.binop.src.treg.sym))
                sprintf(buffer, "qword [rsp]");
            else
                sprintf(buffer, "qword [rsp + %d]",
                  get_stack_offset(inst.binop.src.treg.sym));
            printf("        add     %s, %s\n", inst.binop.dest.treg.sym, buffer);
        }
        
        if(i == acc.count - 2){
            printf("        add     rsp, %d\n", n * 8);
        }
      }
    }
    if (inst.binop.dest.type == OP_REG && inst.binop.src.type == OP_IMM){
        printf("        mov     %s, %d\n", inst.binop.dest.treg.sym, inst.binop.src.imm);
    }
    else if(inst.type == INST_RET){
        
        printf("        ret\n");
    }
  }
}
