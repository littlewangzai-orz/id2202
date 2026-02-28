#include "parser.hpp"
#include "astnode.hpp"
#include "magic_enum.hpp"
#include "scanner.hpp"
#include <fstream>
#include <stdexcept>
#include <variant>
#include <queue>

std::queue<Token> tokenize(std::ifstream &code,bool report = true) {
  scanner s(code);
  std::queue<Token> tokens{};
  while (true) {
    try {
      Token token = s.lex();
      tokens.push(token);
      if (token.token_t == token_type::END_OF_FILE) {
        break;
      }
    } catch (std::runtime_error &err) {
      if (report){
        if (LINE_ERR_ONLY){
          std::string err_msg = err.what();
          auto line_comma = err_msg.rfind(':');
          //"Lexing Error on " is 16 chars long
          std::cerr << err_msg.substr(15,line_comma-15) << std::endl;
        }else{
          std::cerr << err.what() << std::endl;
        }
      }
      exit(1);
    }
  }
  return tokens;
}

void print_type(const Parser &p, node_idx g_idx) {
  auto type_node = p.get_node(g_idx);

  try {
    auto ttype = std::get<TType>(type_node.type);
    std::cout << magic_enum::enum_name(ttype);

    if (ttype == TType::TIdent) {
      std::cout << "(\"" << std::get<std::string>(type_node.primary_val) << "\")";
    }

    if (ttype == TType::TPoint) {
      std::cout << "(";
      print_type(p, std::get<node_idx>(type_node.primary_val));
      std::cout << ")";
    }
  } catch (std::bad_variant_access &err) {
    throw std::runtime_error("Failure printing Type node");
  }
}

void print_expression(const Parser &p,node_idx e_idx){
  auto expr_node = p.get_node(e_idx);
  try {
    EType t = std::get<EType>(expr_node.type);
    std::cout << magic_enum::enum_name(t) << "(";
    switch (t){
    case EType::EInt:
      std::cout <<std::get<int>(expr_node.primary_val);
      break;
    case EType::EChar:
      std::cout << "'" <<std::get<std::string>(expr_node.primary_val) << "'";
      break;
    case EType::EVar:
    case EType::EString:
      std::cout << '"' <<std::get<std::string>(expr_node.primary_val) << '"';
      break;
    case EType::EBinOp: {
      Operator op = std::get<Operator>(expr_node.primary_val);
      for (auto pair: BOP){
        if (pair.second == op){
          std::cout << pair.first << ",";
          print_expression(p,std::get<node_idx>(expr_node.secondary_val));
          std::cout << ",";
          print_expression(p,std::get<node_idx>(expr_node.tertiary_val));
          break;
        }
      }
      break;
    }
    case EType::EUnOp: {
      Operator op = std::get<Operator>(expr_node.primary_val);
      for (auto pair: UOP){
        if (pair.second == op){
          std::cout << pair.first << ",";
          print_expression(p,std::get<node_idx>(expr_node.secondary_val));
          break;
        }
      }
      break;
    }
    case EType::ECall:{
        std::cout << '"' << std::get<std::string>(expr_node.primary_val) << "\",{";
        for (node_idx node : std::get<std::vector<node_idx>>(expr_node.secondary_val)){
          print_expression(p,node);
        }
        std::cout << "}";
        break;
      }
    case EType::ENew:{
        print_type(p,std::get<node_idx>(expr_node.primary_val));
        std::cout << ",";
        print_expression(p,std::get<node_idx>(expr_node.secondary_val));
        break;
      }
    case EType::EArrayAccess: {
        std::cout<< '"' << std::get<std::string>(expr_node.primary_val) << "\",";
        print_expression(p,std::get<node_idx>(expr_node.secondary_val));
        std::cout << ",";
        if (std::holds_alternative<std::string>(expr_node.tertiary_val)){
          std::cout << '"' << std::get<std::string>(expr_node.tertiary_val) << '"';
        }
        break;
      }
    }
    std::cout << ")";

  } catch (std::bad_variant_access& ex){
    throw std::runtime_error(ex.what());
  }
};

void print_statement(const Parser &p,node_idx s_idx){
  auto stmt_node = p.get_node(s_idx);
  try {
    SType t = std::get<SType>(stmt_node.type);
    if (t == SType::SScope){
      std::cout << "\n";
    }
    std::cout << magic_enum::enum_name(t);
    if (t != SType::SBreak){
      std::cout << "(";
    }
    switch (t){
    case SType::SExpr:
      print_expression(p,std::get<node_idx>(stmt_node.primary_val));
      break;
    case SType::SVarDef:{
      print_type(p,std::get<node_idx>(stmt_node.primary_val));
      std::cout << ",\"" << std::get<std::string>(stmt_node.secondary_val) << "\",";
      print_expression(p,std::get<node_idx>(stmt_node.tertiary_val));
      break;
      }
    case SType::SVarAssign: {
      std::cout<< '"' << std::get<std::string>(stmt_node.primary_val) << "\",";
      print_expression(p,std::get<node_idx>(stmt_node.secondary_val));
      break;
      }
    case SType::SArrayAssign: {
      std::cout<< '"' << std::get<std::string>(stmt_node.primary_val) << "\",";
      print_expression(p,std::get<node_idx>(stmt_node.secondary_val));
      std::cout << ",";
      if (std::holds_alternative<std::string>(stmt_node.tertiary_val)){
        std::cout<< '"' << std::get<std::string>(stmt_node.tertiary_val) << "\",";
        print_expression(p,std::get<node_idx>(stmt_node.quad_val));
      }else{
        std::cout << ",";
        print_expression(p,std::get<node_idx>(stmt_node.tertiary_val)); 
      }
      break;
      }
    case SType::SScope: {
        std::cout << "{\n";
        for (node_idx node : std::get<std::vector<node_idx>>(stmt_node.primary_val)){
          print_statement(p,node);
          std::cout << "\n";
        }
        std::cout << "\n}";
        break;
      }
    case SType::SIf: {
        print_expression(p,std::get<node_idx>(stmt_node.primary_val));
        std::cout << ",\n";
        print_statement(p,std::get<node_idx>(stmt_node.secondary_val));
        std::cout << ",";
        if (std::holds_alternative<node_idx>(stmt_node.tertiary_val)){
          std::cout << "\n";
          print_statement(p,std::get<node_idx>(stmt_node.tertiary_val));
        }
        break;
      }
    case SType::SWhile:{
        print_expression(p,std::get<node_idx>(stmt_node.primary_val));
        std::cout << ",\n";
        print_statement(p,std::get<node_idx>(stmt_node.secondary_val));
        break;
      }
    case SType::SBreak:
      break;
    case SType::SReturn:{
      if (std::holds_alternative<node_idx>(stmt_node.primary_val)){
        print_expression(p,std::get<node_idx>(stmt_node.primary_val));
      }
      break;
    }
    case SType::SDelete:
      std::cout << '"' <<std::get<std::string>(stmt_node.primary_val)<< '"';
      break;
    }

    if (t != SType::SBreak){
      std::cout << ")";
    }
  } catch (std::bad_variant_access& ex){
    throw std::runtime_error(ex.what());
  }
  
};

void print_params(const Parser &p, const std::vector<Parameter> &params) {
  std::cout << "{";
  for (auto &param : params) {
    std::cout << "(";
    print_type(p, param.type);
    std::cout << ",\"" << param.name << "\")";
  }

  std::cout << "}";
}

void print_global(const Parser &p, node_idx g_idx) {
  auto global_node = p.get_node(g_idx);
  try {
    auto gtype = std::get<GType>(global_node.type);
    std::cout << magic_enum::enum_name(gtype) << "(";
    if (gtype == GType::GStruct) {
      std::cout << '"' << std::get<std::string>(global_node.primary_val) << "\",";
      auto params = std::get<std::vector<Parameter>>(global_node.secondary_val);
      print_params(p, params);
      std::cout << ")";
      return;
    }
    print_type(p, std::get<node_idx>(global_node.primary_val));
    std::cout << ",\"" << std::get<std::string>(global_node.secondary_val) << "\"";
    switch (gtype) {
    case GType::GFuncDef:
      std::cout << ",";
      print_params(p,
                   std::get<std::vector<Parameter>>(global_node.tertiary_val));
      std::cout << ",";
      print_statement(p,std::get<node_idx>(global_node.quad_val));
      std::cout << ")";
      break;
    case GType::GFuncDecl:
      std::cout << ",";
      print_params(p,
                   std::get<std::vector<Parameter>>(global_node.tertiary_val));
      std::cout << ")";
      break;
    case GType::GVarDef:
      std::cout << ",";
      print_expression(p,std::get<node_idx>(global_node.tertiary_val));
      std::cout << ")";
      break;
    case GType::GVarDecl:
      std::cout << ")";
      break;
    case GType::GStruct:
      break;
    }

  } catch (std::bad_variant_access &err) {
    throw std::runtime_error(err.what());
  }
}

void print_ast(const Parser &p, const std::vector<node_idx> &global_nodes) {
  for (auto g : global_nodes) {
    print_global(p, g);
    std::cout << "\n\n";
  }
}
