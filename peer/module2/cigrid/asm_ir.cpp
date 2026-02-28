
#include "asm_ir.hpp"
#include "astnode.hpp"
#include "ir.hpp"
#include "magic_enum.hpp"
#include <cctype>
#include <stdexcept>
#include <string_view>
#include <variant>

// using env_t = std::map<std::string, std::pair<Register, node_idx>>;
using env_t = std::map<std::string, Register>;

Register expr_instructions(const IRRepr &repr, std::vector<AsmInst> &instrs,
                           node_idx expr_idx, env_t &env, int &tmp_regs);

Register handle_binops(const IRRepr &repr, std::vector<AsmInst> &instrs,
                       ASTNode &expr, env_t &env, int &tmp_regs) {
  Operator op = std::get<Operator>(expr.primary_val);
  switch (op) {
  case Operator::BPlus:
  case Operator::BMinus:
  case Operator::BBitAnd:
  case Operator::BBitOr:
  case Operator::BShLeft:
  case Operator::BShRight: {
    auto lhs_idx = std::get<node_idx>(expr.secondary_val);
    Register lhs = expr_instructions(repr, instrs, lhs_idx, env, tmp_regs);
    auto rhs_idx = std::get<node_idx>(expr.tertiary_val);
    Register rhs = expr_instructions(repr, instrs, rhs_idx, env, tmp_regs);
    AsmOp asm_op;
    switch (op) {
    case Operator::BPlus:
      asm_op = AsmOp::Add;
      break;
    case Operator::BMinus:
      asm_op = AsmOp::Sub;
      break;
    case Operator::BBitAnd:
      asm_op = AsmOp::And;
      break;
    case Operator::BBitOr:
      asm_op = AsmOp::Or;
      break;
    case Operator::BShLeft:
      asm_op = AsmOp::Shl;
      break;
    case Operator::BShRight:
      asm_op = AsmOp::Shr;
      break;
    default:
      throw std::runtime_error("faulty BOP construction");
    }
    AsmInst inst(asm_op, lhs, rhs);
    instrs.push_back(inst);
    return lhs;
  } break;
  case Operator::BMul:
  case Operator::BDiv:
  case Operator::BMod: {
    // ensure rdx is clear

    auto lhs_idx = std::get<node_idx>(expr.secondary_val);
    Register lhs = expr_instructions(repr, instrs, lhs_idx, env, tmp_regs);

    
    auto rhs_idx = std::get<node_idx>(expr.tertiary_val);
    Register rhs = expr_instructions(repr, instrs, rhs_idx, env, tmp_regs);

    // Move lhs to rax
    Register rax("rax");
    AsmInst mv_rax(AsmOp::Mov, rax, lhs);
    instrs.push_back(mv_rax);

    int rst_val = 0;
    Register rdx("rdx");
    AsmInst rst_rdx(AsmOp::Mov, rdx, rst_val);
    instrs.push_back(rst_rdx);

    Register new_tmp(tmp_regs);
    tmp_regs += 1;

    if (op == Operator::BMul) {
      AsmInst mul_inst(AsmOp::IMul, rhs);
      instrs.push_back(mul_inst);
      instrs.push_back(AsmInst(AsmOp::Mov,new_tmp,rax));
      return new_tmp;
    } else if (op == Operator::BDiv) {
      AsmInst div_inst(AsmOp::IDiv, rhs);
      instrs.push_back(div_inst);
      instrs.push_back(AsmInst(AsmOp::Mov,new_tmp,rax));
      return new_tmp;
    } else {
      AsmInst div_inst(AsmOp::IDiv, rhs);
      instrs.push_back(div_inst);
      instrs.push_back(AsmInst(AsmOp::Mov,new_tmp,rdx));
      return new_tmp;
    }
  } break;
    break;
  case Operator::BLt:
  case Operator::BGt:
  case Operator::BLeq:
  case Operator::BGeq:
  case Operator::BEq:
  case Operator::BNeq: {
    // result reg 1 or 0
    Register new_tmp(tmp_regs);
    Register rax("rax");
    Register r10("r10");
    instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, 0));
    tmp_regs += 1;

    auto lhs_idx = std::get<node_idx>(expr.secondary_val);
    Register lhs = expr_instructions(repr, instrs, lhs_idx, env, tmp_regs);
    auto rhs_idx = std::get<node_idx>(expr.tertiary_val);
    Register rhs = expr_instructions(repr, instrs, rhs_idx, env, tmp_regs);
    AsmInst inst(AsmOp::Cmp, lhs, rhs);
    instrs.push_back(inst);
    AsmOp asm_op;
    switch (op) {
    case Operator::BLt:
      asm_op = AsmOp::cMovl;
      break;
    case Operator::BGt:
      asm_op = AsmOp::cMovg;
      break;
    case Operator::BLeq:
      asm_op = AsmOp::cMovle;
      break;
    case Operator::BGeq:
      asm_op = AsmOp::cMovge;
      break;
    case Operator::BEq:
      asm_op = AsmOp::cMove;
      break;
    case Operator::BNeq:
      asm_op = AsmOp::cMovne;
      break;
    default:
      throw std::runtime_error("faulty BOP construction");
    }
    instrs.push_back(AsmInst(AsmOp::Mov, rax, 0));
    instrs.push_back(AsmInst(AsmOp::Mov, r10, 1));
    instrs.push_back(AsmInst(asm_op, rax, r10));
    instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, rax));
    return new_tmp;
  } break;
  case Operator::BAnd:
  case Operator::BOr:
    throw std::runtime_error("Bool ops not implemented");
    break;
  case Operator::UNeg:
  case Operator::UNot:
  case Operator::UMinus:
    throw std::runtime_error("Uop in Bop asm construction");
    break;
  }
  throw std::runtime_error("Unreachable");
}

Register expr_instructions(const IRRepr &repr, std::vector<AsmInst> &instrs,
                           node_idx expr_idx, env_t &env, int &tmp_regs) {

  ASTNode expr = repr.get_node(expr_idx);
  EType type = std::get<EType>(expr.type);
  switch (type) {
  case EType::EVar: {
    std::string var_name = std::get<std::string>(expr.primary_val);
    Register new_tmp(tmp_regs);
    instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, env[var_name]));
    tmp_regs += 1;
    return new_tmp;
  } break;
  case EType::EInt: {
    Register new_tmp(tmp_regs);
    int val = std::get<int>(expr.primary_val);
    instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, val));
    tmp_regs += 1;
    return new_tmp;
  } break;
  case EType::EBinOp: {
    return handle_binops(repr, instrs, expr, env, tmp_regs);
  } break;
  case EType::EUnOp: {
    Operator op = std::get<Operator>(expr.primary_val);
    AsmOp op_t;
    switch (op) {
      // Note strange flip in naming
    case Operator::UNeg:
      op_t = AsmOp::Not;
      break;
    case Operator::UMinus:
      op_t = AsmOp::Neg;
      break;
    case Operator::UNot:
      // Bool case not handled
    default:
      throw std::runtime_error("Uop with faulty operator!");
      break;
    }

    auto rhs_expr = std::get<node_idx>(expr.secondary_val);
    Register res_reg = expr_instructions(repr, instrs, rhs_expr, env, tmp_regs);
    AsmInst uop(op_t, res_reg);
    instrs.push_back(uop);
    return res_reg;
  } break;
  case EType::EChar: {
    Register al{"al"};
    Register rax{"rax"};
    Register new_tmp(tmp_regs);
    auto s = std::get<std::string>(expr.primary_val);
    instrs.push_back(AsmInst(AsmOp::Mov, al, s));
    instrs.push_back(AsmInst(AsmOp::Movsx, rax, al));
    instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, rax));
    tmp_regs += 1;
    return new_tmp;
  } break;
  case EType::ECall: {
    auto func_name = std::get<std::string>(expr.primary_val);
    std::optional<IRGlobal> func_def;
    for (auto d : repr.externs()) {
      if (d.handle == func_name) {
        func_def = d;
      }
    }
    if (!func_def.has_value()) {
      throw std::runtime_error("Calling function without external definition!");
    }

    auto args = std::get<std::vector<node_idx>>(expr.secondary_val);
    std::string arg_regs[6]{"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

    for (size_t a = 0; a < args.size(); ++a) {
      Register reg{arg_regs[a]};
      Register res_reg =
          expr_instructions(repr, instrs, args[a], env, tmp_regs);
      auto type_node = repr.get_node(func_def.value().params[a].type);
      if (std::get<TType>(type_node.type) == TType::TChar) {
        res_reg.size = RegSize::byte;
      }
      instrs.push_back(AsmInst(AsmOp::Mov, reg, res_reg));
    }
    instrs.push_back(AsmInst(AsmOp::Push, Register{"rsp"}));
    instrs.push_back(AsmInst(AsmOp::Call, func_name));

    auto type_node = repr.get_node(func_def.value().ret_type);
    auto ttype = std::get<TType>(type_node.type);
    Register rax{"rax"};
    instrs.push_back(AsmInst(AsmOp::Pop, Register{"rsp"}));

    if (ttype == TType::TInt || ttype == TType::TPoint) {
      Register new_tmp(tmp_regs);
      instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, Register{"rax"}));
      tmp_regs += 1;
      return new_tmp;
    } else if (ttype == TType::TChar) {

      Register new_tmp(tmp_regs);
      Register al{"al"};
      instrs.push_back(AsmInst(AsmOp::Movsx, rax, al));
      instrs.push_back(AsmInst(AsmOp::Mov, new_tmp, rax));
      tmp_regs += 1;
      return new_tmp;
    }
    return rax;
  } break;
  case EType::EString:
  case EType::ENew:
  case EType::EArrayAccess:
    throw std::runtime_error("Not implemented Asm Expr");
    break;
  }
  throw std::runtime_error("Unreachable");
}

AsmBlock make_asmblk(const IRRepr &repr, const IRBlock &blk, env_t &env,
                     int &reg_cnt) {
  std::vector<AsmInst> instrs;
  for (auto &stmt : blk.statements) {
    switch (stmt.type) {
    case Stmt_t::IRExpr: {

      auto expr_res =
          expr_instructions(repr, instrs, stmt.node_ref.value(), env, reg_cnt);
    } break;
    case Stmt_t::IRVarDecl: {
      std::string var_name = stmt.key.value();
      Register new_reg(reg_cnt, var_name.c_str());
      env[var_name] = new_reg;
      reg_cnt += 1;
    } break;
    case Stmt_t::IRVarAssign: {
      if (env.find(stmt.key.value()) == env.end()) {
        throw std::runtime_error("Var assigned but not declared");
      }
      int tmp_reg = reg_cnt;
      auto rhs_reg =
          expr_instructions(repr, instrs, stmt.node_ref.value(), env, tmp_reg);
      AsmInst assignment = AsmInst(AsmOp::Mov, env[stmt.key.value()], rhs_reg);
      instrs.push_back(assignment);
    } break;
    }
  }

  IRBranch brch = blk.branch;
  switch (brch.type) {
  case Branch_t::Return: {
    if (brch.expression.has_value()) {
      int tmp_reg = reg_cnt;
      auto rhs_reg = expr_instructions(repr, instrs, brch.expression.value(),
                                       env, tmp_reg);
      Register ret_reg("rax");
      AsmInst ret_mov(AsmOp::Mov, ret_reg, rhs_reg);
      instrs.push_back(ret_mov);
    }
    AsmInst ret(AsmOp::Ret);
    instrs.push_back(ret);

    return AsmBlock(blk.name, std::move(instrs), ret);
  } break;
  case Branch_t::Branch: {
    int tmp_reg = reg_cnt;
    Register cond_res =
        expr_instructions(repr, instrs, brch.expression.value(), env, tmp_reg);
    AsmInst comp(AsmOp::Cmp, cond_res, 1);
    instrs.push_back(comp);
    AsmInst j(AsmOp::Je, brch.primary_target.value());
    instrs.push_back(j);
    j = AsmInst(AsmOp::Jmp, brch.secondary_target.value());
    instrs.push_back(j);

    return AsmBlock(blk.name, std::move(instrs), j);
  } break;
  case Branch_t::Jump: {
    AsmInst j(AsmOp::Jmp, brch.primary_target.value());
    instrs.push_back(j);
    return AsmBlock(blk.name, std::move(instrs), j);
  } break;
  }
  throw std::runtime_error("Unreachable");
}

std::vector<AsmLabel> instruction_selection(IRRepr representation) {
  std::vector<AsmLabel> procedures;

  for (auto &ext : representation.externs()) {
    procedures.push_back(AsmLabel(ext.handle));
  }

  for (auto &global : representation.global_blks()) {
    if (!global.params.empty()) {
      throw std::runtime_error("Function params not implemented");
    }

    TType ret_type =
        std::get<TType>(representation.get_node(global.ret_type).type);
    if (ret_type != TType::TInt && ret_type != TType::TVoid) {
      throw std::runtime_error("Only void and int return type supported");
    }

    std::vector<AsmBlock> asm_blks;

    env_t env{};
    int reg_cnt = 1;
    for (auto &blk : global.blocks) {
      asm_blks.push_back(make_asmblk(representation, blk, env, reg_cnt));
    }

    procedures.push_back(AsmLabel(global.handle, std::move(asm_blks)));
  }

  return procedures;
};

std::string format_inst_arg(AsmInstArg arg) {

  if (std::holds_alternative<std::string>(arg)) {
    std::stringstream ss;
    std::string s = std::get<std::string>(arg);
    if (s == "\\'"){
      return "\"'\"";
    }else if (s == "\\\""){
       return "\'\"\'";     
    }else if (s == "\\n"){
      return "0x0A";
    }else if (s == "\\\\"){
      return "\'\\\'";
    }else if (s == "\\t"){
      return "0x9";
    }
    ss << "'" << s << "'";
    return ss.str();
  }

  if (std::holds_alternative<int>(arg)) {
    std::stringstream ss;
    ss << std::get<int>(arg);
    return ss.str();
  }
  
  if (std::holds_alternative<Register>(arg)) {
    auto reg = std::get<Register>(arg);
    return reg.name;
  }

  if (std::holds_alternative<AsmMemRef>(arg)) {
    std::stringstream ss;
    auto mem_ref = std::get<AsmMemRef>(arg);

    ss << magic_enum::enum_name(mem_ref.size);

    ss << "[ ";
    if (mem_ref.reg_base.has_value()) {
      ss << mem_ref.reg_base.value().name << " ";
    }

    if (mem_ref.reg_add.has_value()) {
      ss << "+ " << mem_ref.reg_add.value().name << " ";

      // only allow scaler if on reg?
      if (mem_ref.scale_reg.has_value()) {
        ss << "* " << mem_ref.scale_reg.value() << " ";
      }
    }

    if (mem_ref.offset.has_value()) {
      if (mem_ref.reg_base.has_value()) {
        ss << "+ ";
      }
      ss << mem_ref.offset.value();
    }
    ss << "]";
    return ss.str();
  }

  throw std::runtime_error("Unreachable");
}

void print_asm(std::vector<AsmLabel> &labels) {
  for (const auto &label : labels) {
    if (label.external) {
      continue;
    }
    std::cout << "\t global " << label.label << "\n\n";
  }

  for (const auto &label : labels) {
    if (label.external) {
      std::cout << "\t extern " << label.label << "\n\n";
    }
  }

  for (auto label : labels) {
    if (label.external) {
      continue;
    }
    std::cout << "\t\t\t ; function " << label.label << "\n";

    std::cout << label.label << ": \n";

    for (auto blk : label.blocks) {
      std::cout << "$" << blk.name << ":\n";
      for (auto inst : blk.instructions) {
        std::string op_str{magic_enum::enum_name(inst.op_class)};
        std::transform(op_str.begin(), op_str.end(), op_str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        std::cout << "\t" << op_str;

        if (inst.jmp_label.has_value()) {

          std::cout << " $"<< inst.jmp_label.value();
        }

        if (inst.op1.has_value()) {
          std::cout << " " << format_inst_arg(inst.op1.value());
        }

        if (inst.op2.has_value()) {
          std::cout << ", " << format_inst_arg(inst.op2.value());
        }
        std::cout << "\n";
      }
    }
  }
}

std::optional<AsmMemRef> add_regs(std::map<std::string, AsmMemRef> &used_regs,
                                  AsmInstArg arg, int &stack_offset) {
  if (std::holds_alternative<Register>(arg)) {
    Register reg = std::get<Register>(arg);
    if (!reg.isTemp) {
      return std::nullopt;
    }
    if (used_regs.find(reg.name) == used_regs.end()) {
      Register rsp{"rsp"};
      AsmMemRef mem{reg.size, rsp};
      if (stack_offset != 0) {
        mem.offset = stack_offset;
      }
      used_regs[reg.name] = mem;
      stack_offset += reg.size / RegSize::byte;
      return mem;
    } else {
      return used_regs[reg.name];
    }
  }
  return std::nullopt;
}

void spill(std::vector<AsmLabel> &labels) {
  std::map<std::string, AsmMemRef> used_regs;
  int stack_offset = 0;
  for (auto &label : labels) {
    if (label.blocks.empty()) {
      continue;
    }
    for (auto &blk : label.blocks) {
      std::vector<AsmInst> new_inst;
      for (auto &inst : blk.instructions) {
        std::optional<AsmMemRef> opt1;
        if (inst.op1.has_value()) {
          opt1 = add_regs(used_regs, inst.op1.value(), stack_offset);
          if (opt1.has_value()) {
            inst.op1 = opt1.value();
          }
        }

        if (inst.op2.has_value()) {
          auto opt = add_regs(used_regs, inst.op2.value(), stack_offset);
          if (opt.has_value()) {
            inst.op2 = opt.value();
            if (opt1.has_value()) {
              Register r10{"r10"};
              AsmInst movtoreg{AsmOp::Mov, r10, opt.value()};
              new_inst.push_back(movtoreg);
              inst.op2 = r10;
            }
          }
        }
        new_inst.push_back(inst);
      }
      blk.instructions = std::move(new_inst);
      auto brch_arg1 = blk.branch_inst.op1;
      if (brch_arg1.has_value()) {
        auto opt = add_regs(used_regs, brch_arg1.value(), stack_offset);
        if (opt.has_value()) {
          blk.branch_inst.op1 = opt.value();
        }
      }

      auto brch_arg2 = blk.branch_inst.op2;
      if (brch_arg2.has_value()) {
        auto opt = add_regs(used_regs, brch_arg2.value(), stack_offset);
        if (opt.has_value()) {
          blk.branch_inst.op2 = opt.value();
        }
      }
    }

    Register rsp{"rsp"};
    AsmInst sub_stack{AsmOp::Sub, rsp, stack_offset};
    AsmInst add_stack{AsmOp::Add, rsp, stack_offset};
    label.blocks.front().instructions.insert(
        label.blocks.front().instructions.begin(), sub_stack);

    for (auto &blk : label.blocks) {
      if (blk.branch_inst.op_class == AsmOp::Ret ||
          blk.name == label.blocks.back().name) {

        blk.instructions.insert(std::prev(blk.instructions.end()), add_stack);
      }
    }
  }
}
