#ifndef ASM_IR_HPP
#define ASM_IR_HPP
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include "ir.hpp"

enum AsmOp: int {
  Dec,
  Inc,
  Div,
  IDiv,
  Mul,
  IMul,
  Neg,
  Not,
  Push,
  Pop,
  Setg,
  Setge,
  Sete,
  Setne,
  Setl,
  Setle, // Uop -> Bop
  And,
  Or,
  Xor,
  Cmp,
  cMovl,
  cMovle,
  cMove,
  cMovne,
  cMovg,
  cMovge,
  Mov,
  Movsx,
  Movzx,
  Add,
  Sub,
  Sar,
  Shl,
  Shr,
  Xchg,
  Cqo, // Bop -> Noop
  Cdq,
  Noop,
  Jmp, // JmpOps
  Je,
  Jne,
  Jge,
  Jg,
  Jl,
  Jle,
  Call,
  Ret,
};

static const int ASM_UOP_MAX = 15;
static const int ASM_BOP_MAX = 34;
static const int ASM_NOOP_MAX = 37;


enum RegSize: int{
  byte = 8,
  word = 16,
  dword = 32,
  qword = 64,
};

struct Register{
  bool isTemp;
  std::string name;
  int rCnt;
  RegSize size;

  Register(): isTemp(true),name("Invalid"),rCnt(-1),size(RegSize::byte){}
  
  Register(int cnt, const char* prefix= "tmp_", RegSize sz = RegSize::qword): isTemp(true),rCnt(cnt),size(sz){
    std::stringstream ss;
    ss << prefix << cnt;
    name = ss.str();
  }

  Register(std::string reg_name):isTemp(false),name(reg_name),rCnt(0){
    if (reg_name.back() == 'w'){
      size = RegSize::word;
    }else if (reg_name.back() == 'd'){
      size = RegSize::dword;
    }else if (reg_name.back() == 'b' || reg_name.back() == 'l'){
      size = RegSize::byte;
    }else if (reg_name.front() == 'e'){
      size = RegSize::dword;
    }else if (reg_name.front() == 'r'){
      size = RegSize::qword;
    }else{
      size = RegSize::word;
    }
  }
};

struct AsmMemRef {
  RegSize size = RegSize::qword;
  std::optional<Register> reg_base;
  std::optional<Register> reg_add;
  std::optional<int> scale_reg;
  std::optional<int> offset;
};

using AsmInstArg = std::variant<Register,int,std::string,AsmMemRef>;

class AsmInst {

public:
  AsmOp op_class;
  std::optional<AsmInstArg> op1;
  std::optional<AsmInstArg> op2;
  std::optional<std::string> jmp_label;

  AsmInst(AsmOp op): op_class(op){
    if (op != AsmOp::Ret && ( op <= ASM_BOP_MAX || op > ASM_NOOP_MAX)){
      throw std::runtime_error("Only returns and Noops have no operands");
    }
  }

  AsmInst(AsmOp op, AsmInstArg arg1): op_class(op),op1(arg1){
    if (op > ASM_UOP_MAX){
      throw std::runtime_error("Only Uops have one operand");
    }
  }

  AsmInst(AsmOp op, std::string label): op_class(op),jmp_label(label){
    if (op < ASM_NOOP_MAX || op == AsmOp::Ret){
      throw std::runtime_error("Only jumps excluding ret have a label operand");
    }
  }  

  AsmInst(AsmOp op, AsmInstArg arg1, AsmInstArg arg2): op_class(op), op1(arg1),op2(arg2){
    if (op <= ASM_UOP_MAX || op > ASM_BOP_MAX) {
      throw std::runtime_error("Only Bops can have 2 operands operands");
    }
  }
};

struct AsmBlock{
  std::string name;
  std::vector<AsmInst> instructions;
  AsmInst branch_inst;

  AsmBlock(std::string blk_name,std::vector<AsmInst>&& instrs, AsmInst branching): name(blk_name),instructions(instrs), branch_inst(branching){
    if (branching.op_class <= ASM_NOOP_MAX){
      throw std::runtime_error("branching statement for block not a jmp!");
    }
  };
};


struct AsmLabel{
  std::string label; 
  std::vector<AsmBlock> blocks;
  bool external = false;

  AsmLabel(std::string func_label,std::vector<AsmBlock>&& blks): label(func_label),blocks(blks){
  }

  AsmLabel(std::string func_label): label(func_label),blocks({}),external(true){
  }
};

std::vector<AsmLabel> instruction_selection(IRRepr representation);


void print_asm(std::vector<AsmLabel>& labels);

void spill(std::vector<AsmLabel>& labels);


#endif
