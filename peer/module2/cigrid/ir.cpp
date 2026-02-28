#include "ir.hpp"
#include "astnode.hpp"
#include "parser.hpp"
#include <sstream>
#include <stdexcept>
#include <variant>

IRStmt IRRepr::make_expr(node_idx sexpr) {
  ASTNode expr_node = parse_tree.get_node(sexpr);
  if (!std::holds_alternative<EType>(expr_node.type)) {
    throw std::runtime_error("IRExpr only holds expr nodes");
  }
  return IRStmt(Stmt_t::IRExpr, sexpr);
}

IRStmt IRRepr::make_vardecl(std::string var_name, node_idx type_idx) {
  ASTNode type_node = parse_tree.get_node(type_idx);
  if (!std::holds_alternative<TType>(type_node.type)) {
    throw std::runtime_error("Var decl requires type node ref");
  }

  return IRStmt(Stmt_t::IRVarDecl, var_name, type_idx);
}

IRStmt IRRepr::make_varassign(std::string var_name, node_idx rhs_expr) {
  ASTNode expr_node = parse_tree.get_node(rhs_expr);
  if (!std::holds_alternative<EType>(expr_node.type)) {
    throw std::runtime_error("IRExpr only holds expr nodes");
  }
  return IRStmt(Stmt_t::IRVarAssign, var_name, rhs_expr);
}

std::vector<IRBlock>
IRRepr::construct_scope(node_idx scope_idx,
                        std::optional<std::string> exit_blk_key) {
  ASTNode scope = parse_tree.get_node(scope_idx);
  if (std::get<SType>(scope.type) != SType::SScope)
    throw std::runtime_error("Expected scope");

  auto stmt_nodes = std::get<std::vector<node_idx>>(scope.primary_val);
  return parse_scope_stmts(stmt_nodes, exit_blk_key);
}

std::vector<IRBlock>
IRRepr::parse_scope_stmts(std::vector<node_idx> stmt_nodes,
                          std::optional<std::string> exit_blk_key) {
  std::vector<IRBlock> blocks;
  std::vector<IRStmt> current_blk;

  std::stringstream ss;
  ss << "scope_" << current_blk_key;
  std::string curr_scope = ss.str();
  current_blk_key += 1;

  for (auto s_idx : stmt_nodes) {
    ASTNode statement = parse_tree.get_node(s_idx);
    assert(std::holds_alternative<SType>(statement.type));
    switch (std::get<SType>(statement.type)) {
    case SType::SExpr: {
      auto stmt = make_expr(std::get<node_idx>(statement.primary_val));
      current_blk.push_back(stmt);
    } break;
    case SType::SVarDef: {
      auto type_idx = std::get<node_idx>(statement.primary_val);
      auto var_name = std::get<std::string>(statement.secondary_val);
      auto rhs_expr = std::get<node_idx>(statement.tertiary_val);
      // Split into decl and assign
      // Only allow primitives
      ASTNode type_node = parse_tree.get_node(type_idx);
      TType var_type = std::get<TType>(type_node.type);
      if (var_type != TType::TInt && var_type != TType::TChar) {
        throw std::runtime_error("Var decl only implemented for int and char");
      }

      IRStmt var_decl = make_vardecl(var_name, type_idx);
      current_blk.push_back(var_decl);
      IRStmt var_val = make_varassign(var_name, rhs_expr);
      current_blk.push_back(var_val);
    } break;
    case SType::SVarAssign: {
      auto var_name = std::get<std::string>(statement.primary_val);
      auto rhs_expr = std::get<node_idx>(statement.secondary_val);
      IRStmt var_val = make_varassign(var_name, rhs_expr);
      current_blk.push_back(var_val);
    } break;
    case SType::SReturn: {

      if (std::holds_alternative<node_idx>(statement.primary_val)) {
        node_idx expr_idx = std::get<node_idx>(statement.primary_val);

        ASTNode expr_node = parse_tree.get_node(expr_idx);
        if (!std::holds_alternative<EType>(expr_node.type)) {
          throw std::runtime_error("Returns only allows expressions");
        }
        IRBranch returnstmt(Branch_t::Return, expr_idx);
        IRBlock block{curr_scope, std::move(current_blk), returnstmt};
        blocks.push_back(block);
      } else {
        IRBranch returnstmt(Branch_t::Return);
        IRBlock block{curr_scope, std::move(current_blk), returnstmt};
        blocks.push_back(block);
      }

      if (exit_blk_key.has_value()) {
        return blocks;
      }
      current_blk = std::vector<IRStmt>();
      ss.str("");
      ss << "scope_" << current_blk_key;
      curr_scope = ss.str();
      current_blk_key += 1;
    } break;
    case SType::SDelete:
    case SType::SArrayAssign:
      throw std::runtime_error("TODO Not implemented structs/heap/arrays");
      break;
    case SType::SScope: {

      std::string after_scope;
      if (exit_blk_key.has_value()) {
        after_scope = exit_blk_key.value();
      } else {
        ss.str("");
        ss << "scope_" << current_blk_key;
        after_scope = ss.str();
        current_blk_key += 1;
      }

      auto inner_scope = construct_scope(s_idx, after_scope);
      IRBranch returnstmt(Branch_t::Jump, inner_scope.front().name);
      IRBlock block{curr_scope, std::move(current_blk), returnstmt};
      blocks.push_back(block);
      for (auto blk : inner_scope) {
        blocks.push_back(blk);
      }
      current_blk = std::vector<IRStmt>();
      curr_scope = after_scope;

    } break;
    case SType::SIf: {
      node_idx expr_idx = std::get<node_idx>(statement.primary_val);

      ASTNode expr_node = parse_tree.get_node(expr_idx);
      if (!std::holds_alternative<EType>(expr_node.type)) {
        throw std::runtime_error("Returns only allows expressions");
      }

      std::string after_if_scope;
      ss.str("");
      ss << "scope_" << current_blk_key;
      after_if_scope = ss.str();
      current_blk_key += 1;

      auto if_idx = std::get<node_idx>(statement.secondary_val);
      auto if_node = parse_tree.get_node(if_idx);
      std::vector<IRBlock> if_blks;
      if (std::get<SType>(if_node.type) != SType::SScope) {
        if_blks = parse_scope_stmts({if_idx}, after_if_scope);
      } else {
        if_blks = construct_scope(if_idx, after_if_scope);
      }
      std::string if_true_scope = if_blks.front().name;

      std::vector<IRBlock> else_blks;
      if (std::holds_alternative<node_idx>(statement.tertiary_val)) {
        auto else_idx = std::get<node_idx>(statement.tertiary_val);
        auto else_node = parse_tree.get_node(if_idx);
        if (std::get<SType>(else_node.type) != SType::SScope) {
          else_blks = parse_scope_stmts({else_idx}, after_if_scope);
        } else {
          else_blks = construct_scope(else_idx, after_if_scope);
        }
      }
      std::string else_scope =
          else_blks.empty() ? after_if_scope : else_blks.front().name;

      IRBranch returnstmt(Branch_t::Branch, expr_idx, if_true_scope,
                          else_scope);
      IRBlock block{curr_scope, std::move(current_blk), returnstmt};
      blocks.push_back(block);

      current_blk = std::vector<IRStmt>();
      curr_scope = after_if_scope;
      for (auto blk : if_blks) {
        blocks.push_back(blk);
      }

      for (auto blk : else_blks) {
        blocks.push_back(blk);
      }

    } break;
    case SType::SWhile: {

      node_idx expr_idx = std::get<node_idx>(statement.primary_val);

      ASTNode expr_node = parse_tree.get_node(expr_idx);
      if (!std::holds_alternative<EType>(expr_node.type)) {
        throw std::runtime_error("Returns only allows expressions");
      }

      std::string while_test_scope;
      if (current_blk.empty()) {
        while_test_scope = curr_scope;
      } else {
        ss.str("");
        ss << "scope_" << current_blk_key;
        while_test_scope = ss.str();
        current_blk_key += 1;
        IRBranch returnstmt(Branch_t::Jump, while_test_scope);
        IRBlock block{curr_scope, std::move(current_blk), returnstmt};
        blocks.push_back(block);
        current_blk = std::vector<IRStmt>();
      }

      std::string after_if_scope;
      ss.str("");
      ss << "scope_" << current_blk_key;
      after_if_scope = ss.str();
      current_blk_key += 1;

      // construct true scope
      auto if_idx = std::get<node_idx>(statement.secondary_val);
      auto if_node = parse_tree.get_node(if_idx);
      std::vector<IRBlock> if_blks;
      if (std::get<SType>(if_node.type) != SType::SScope) {
        if_blks = parse_scope_stmts({if_idx}, while_test_scope);
      } else {
        if_blks = construct_scope(if_idx, while_test_scope);
      }
      // Make test scope into a if statement
      IRBranch ifstmt(Branch_t::Branch, expr_idx, if_blks.front().name,
                      after_if_scope);
      IRBlock test_blk{while_test_scope, {}, ifstmt};
      blocks.push_back(test_blk);

      for (auto blk : if_blks) {
        blocks.push_back(blk);
      }

      curr_scope = after_if_scope;

    } break;
    case SType::SBreak:
      throw std::runtime_error("TODO Not implemented break");
      break;
    }
  }

  if (!exit_blk_key.has_value()) {
    IRBranch returnstmt(Branch_t::Return);
    IRBlock block{curr_scope, std::move(current_blk), returnstmt};
    blocks.push_back(block);
  } else {
    IRBranch returnstmt(Branch_t::Jump, exit_blk_key.value());
    IRBlock block{curr_scope, std::move(current_blk), returnstmt};
    blocks.push_back(block);
  }

  return blocks;
}

std::vector<IRGlobal> IRRepr::construct_IR() {
  std::vector<IRGlobal> globals;
  for (auto g : global_nodes) {
    // TODO: TEMP For S, ignore all but main

    ASTNode node = parse_tree.get_node(g);
    GType gt = std::get<GType>(node.type);
    if (gt == GType::GFuncDef) {
      if (std::get<std::string>(node.secondary_val) != "main")
        continue;
      node_idx scope = std::get<node_idx>(node.quad_val);
      auto blks = construct_scope(scope);

      node_idx type_idx = std::get<node_idx>(node.primary_val);
      auto params = std::get<std::vector<Parameter>>(node.tertiary_val);

      globals.push_back(IRGlobal{"main", type_idx, params, blks});
    } else if (gt == GType::GFuncDecl) {
      std::string func_name = std::get<std::string>(node.secondary_val);
      node_idx type_idx = std::get<node_idx>(node.primary_val);
      auto params = std::get<std::vector<Parameter>>(node.tertiary_val);
      extern_decls.push_back(IRGlobal{func_name, type_idx, params});
    }
  }
  return globals;
}

void IRRepr::print_stmt(IRStmt stmt) {
  switch (stmt.type) {
  case Stmt_t::IRExpr: {
    std::cout << "IRExpr(";
    print_expression(parse_tree, stmt.node_ref.value());
    std::cout << ")";
  } break;
  case Stmt_t::IRVarDecl: {
    std::cout << "IRVarDecl(";
    print_type(parse_tree, stmt.node_ref.value());
    std::cout << ",\"" << stmt.key.value() << '"';
    std::cout << ")";
  } break;
  case Stmt_t::IRVarAssign: {
    std::cout << "IRVarAssign(\"";
    std::cout << stmt.key.value() << "\",";
    print_expression(parse_tree, stmt.node_ref.value());
    std::cout << ")";
  } break;
  }
}

void IRRepr::print_blk(IRBlock blk, int tab_count) {
  std::string tabs(tab_count, '\t');
  std::cout << "IRBlock(\"" << blk.name << "\",\n" << tabs;

  for (auto stmt : blk.statements) {
    std::cout << "\t";
    print_stmt(stmt);
    std::cout << "\n" << tabs;
  }

  switch (blk.branch.type) {
  case Branch_t::Return: {
    std::cout << "IRReturn(";
    if (blk.branch.expression.has_value()) {
      print_expression(parse_tree, blk.branch.expression.value());
    }
    std::cout << ")";
  } break;
  case Branch_t::Branch: {
    std::cout << "IRBranch(";
    print_expression(parse_tree, blk.branch.expression.value());
    std::cout << ",\"" << blk.branch.primary_target.value() << "\"";
    if (blk.branch.secondary_target.has_value()) {
      std::cout << ",\"" << blk.branch.secondary_target.value() << "\"";
    }
    std::cout << ")";
  } break;
  case Branch_t::Jump: {
    std::cout << "IRJump(";
    std::cout << "\"" << blk.branch.primary_target.value() << "\"";
    std::cout << ")";
  } break;
  }

  std::cout << "\n"
            << tabs << ")"
            << "\n"
            << tabs;
}

void IRRepr::print_IR() {
  for (auto e : extern_decls) {

    std::cout << "IRGlobal(";
    print_type(parse_tree, e.ret_type);
    std::cout << ",\"" << e.handle << "\",";
    print_params(parse_tree, e.params);
    std::cout << ", external)\n";
  }

  for (auto g : globals) {
    std::cout << "IRGlobal(";

    print_type(parse_tree, g.ret_type);
    std::cout << ",\"" << g.handle << "\",";
    print_params(parse_tree, g.params);
    if (!g.blocks.empty()) {
      std::cout << ",\n \t";
      for (auto blk : g.blocks) {
        print_blk(blk, 2);
      }
      std::cout << "\n";
    }
    std::cout << ")\n";
  }
}

const std::vector<IRGlobal> &IRRepr::global_blks() const { return globals; }
const std::vector<IRGlobal> &IRRepr::externs() const { return extern_decls; }
