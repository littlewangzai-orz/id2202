#pragma once 

typedef int Displacement;
typedef int Scale;



typedef enum {
    BYTE,
    WORD,
    DWORD,
    QWORD
} Bitsize;

typedef struct {
    int reg_number;
    Bitsize size;
} Reg;

typedef enum {
    UNOP_INC,    // Increment
    UNOP_DEC,    // Decrement
    UNOP_PUSH,   // Push onto the stack
    UNOP_POP,    // Pop from the stack
    UNOP_IMUL,   // Signed multiplication
    UNOP_IDIV,   // Signed division
    UNOP_NOT,    // Bitwise NOT
    UNOP_NEG,    // Negation
    UNOP_SETG,   // Set greater-than flag
    UNOP_SETL,   // Set less-than flag
    UNOP_SETGE,  // Set greater-than-or-equal flag
    UNOP_SETLE,  // Set less-than-or-equal flag
    UNOP_SETE,   // Set equal flag
    UNOP_SETNE   // Set not-equal flag
} UnOp;

typedef enum {
    BINOP_ADD,   // Addition
    BINOP_SUB,   // Subtraction
    BINOP_CMP,   // Comparison
    BINOP_MOV,   // Data transfer
    BINOP_AND,   // Bitwise AND
    BINOP_OR,    // Bitwise OR
    BINOP_XOR    // Bitwise XOR
} BinOp;

typedef enum {
    OP_IMM,     // Immediate value
    OP_REG,     // General-purpose register
    OP_TREG,    // Virtual register
    OP_MEM,     // Memory address
    OP_NOOP     // No operation
} OperandType;

typedef struct {
    Reg reg;
    char* sym;
} TReg;

typedef struct {
    OperandType type;  // Operand type
    union {
        int imm;       // Immediate value
        Reg reg;       // General-purpose register
        TReg treg;     // Virtual register
        struct {       // Memory address
            Bitsize bitsize;  // Data size
            Reg base;         // Base register
            Reg* index;       // Index register (NULL means no index)
            int scale;        // Scale factor
            int displacement; // Displacement
        } mem;
    };
} Operand;

// Conditional jump operators
typedef enum {
    JBINOP_JL,
    JBINOP_JG,
    JBINOP_JLE,
    JBINOP_JGE,
    JBINOP_JE,
    JBINOP_JNE
} JBinOp;

typedef enum {
    INST_RET,    // Return instruction
    INST_UNOP,   // Unary operation
    INST_BINOP,  // Binary operation
    INST_CALL,   // Function call
    INST_CQO     // Sign extension instruction
} InstructionType;

typedef struct {
    InstructionType type; // Instruction type
    union {
        struct {          // Unary operation
            UnOp op;      // Operator
            Operand dest; // Operand
        } unop;
        struct {          // Binary operation
            BinOp op;     // Operator
            Operand dest; // Destination operand
            Operand src;  // Source operand
        } binop;
        char* call_sym;     // Symbol for function call
        // CQO does not require additional data
    };
} Inst;

// Define block end types
typedef enum {
    BLOCKEND_RET,     // Return
    BLOCKEND_JMP,     // Jump
    BLOCKEND_JBINOP   // Conditional jump
} BlockEndType;

// Define block end structure
typedef struct {
    BlockEndType type; // End type
    union {
        char* jmp_target;      // Jump target
        struct {               // Conditional jump
            JBinOp op;         // Conditional operator
            char* true_target; // Target if condition is true
            char* false_target; // Target if condition is false
        } jbinop;
    };
} BlockEnd;

// Define basic block structure
typedef struct {
    char* sym;              // Basic block name
    Inst* inst;             // List of instructions
    int instruction_count;  // Number of instructions
    BlockEnd end;           // Block end
} Block;

// Define function structure
typedef struct {
    char* sym;     // Function name
    Block* blocks; // List of basic blocks
    int block_count; // Number of basic blocks
} Func;

Operand make_imm_operand(int value);

Operand make_reg_operand(const char* reg_name);

Operand make_treg_operand(TReg reg);

Inst create_binop_instruction(BinOp op_type, Operand dest, Operand src);

Inst create_ret_instruction();

