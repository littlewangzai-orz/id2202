#ifndef IR_HPP
#define IR_HPP
#include "astnode.hpp"
#include "parser.hpp"
#include <optional>
#include <stdexcept>

enum class Stmt_t {
  IRExpr,
  IRVarDecl,
  IRVarAssign,
};

enum class Branch_t {
  Return,
  Branch,
  Jump,
};

class IRBranch {

public:
  Branch_t type;
  std::optional<node_idx> expression; // assert that this is Expression only
  std::optional<std::string> primary_target;
  std::optional<std::string> secondary_target;

  IRBranch(Branch_t t) : type(t) {
    if (t != Branch_t::Return)
      throw std::runtime_error("Invalid init of IRBranch, must be Return");
  };

  IRBranch(Branch_t t, node_idx n) : type(t), expression(n) {
    if (t != Branch_t::Return)
      throw std::runtime_error("Invalid init of IRBranch, must be Return");
  };

  IRBranch(Branch_t t, node_idx n, std::string jmp_opt1, std::string jmp_opt2)
      : type(t), expression(n), primary_target(jmp_opt1),
        secondary_target(jmp_opt2) {
    if (t != Branch_t::Branch)
      throw std::runtime_error("Invalid init of IRBranch, must be Branch");
  };

  IRBranch(Branch_t t, std::string jmp) : type(t), primary_target(jmp) {
    if (t != Branch_t::Jump)
      throw std::runtime_error("Invalid init of IRBranch, must be Jump");
  };
};

class IRStmt {
public:
  Stmt_t type;

  std::optional<std::string> key;
  std::optional<node_idx> node_ref;

  IRStmt(Stmt_t t, node_idx n) : type(t), node_ref(n) {
    if (t != Stmt_t::IRExpr)
      throw std::runtime_error("Invalid init of IRStmt, must be Expr");
  };

  IRStmt(Stmt_t t, std::string k, node_idx var_t)
      : type(t), key(k), node_ref(var_t) {
    if (t == Stmt_t::IRExpr)
      throw std::runtime_error("Invalid init of IRExpr");
  };
};

struct IRBlock {
  std::string name;
  std::vector<IRStmt> statements;
  IRBranch branch;
};

struct IRGlobal {
  std::string handle;
  node_idx ret_type;
  std::vector<Parameter> params;
  std::vector<IRBlock> blocks;
};

class IRRepr {

  int current_blk_key = 1;

  Parser parse_tree;
  std::vector<node_idx> global_nodes;
  std::vector<IRGlobal> globals;
  std::vector<IRGlobal> extern_decls;

  IRStmt make_expr(node_idx sexpr);

  IRStmt make_vardecl(std::string var_name, node_idx type_idx);

  IRStmt make_varassign(std::string var_name, node_idx rhs_expr);

  std::vector<IRBlock>
  construct_scope(node_idx scope_idx,
                  std::optional<std::string> exit_blk_key = std::nullopt);

  std::vector<IRBlock>
  parse_scope_stmts(std::vector<node_idx> stmt_nodes,
                    std::optional<std::string> exit_blk_key= std::nullopt);

  std::vector<IRGlobal> construct_IR();

  void print_stmt(IRStmt stmt);

  void print_blk(IRBlock blk, int tab_count);

public:
  IRRepr(Parser &&p, std::vector<node_idx> &&global_nodes)
      : parse_tree(p), global_nodes(global_nodes) {
    extern_decls = {};
    globals = construct_IR();
  }

  void print_IR();

  const std::vector<IRGlobal> &global_blks() const;

  const std::vector<IRGlobal> &externs() const;

  const ASTNode &get_node(node_idx idx) const {
    return parse_tree.get_node(idx);
  }
};

#endif
