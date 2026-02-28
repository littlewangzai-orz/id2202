#ifndef PARSER_HPP
#define PARSER_HPP

#ifdef LINE_ERR
static const bool LINE_ERR_ONLY = true;
#else
static const bool LINE_ERR_ONLY = true;
#endif
static bool PRINT_ERR = false;

#include "astnode.hpp"
#include "magic_enum.hpp"
#include "scanner.hpp"
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <variant>

class Parser {
  public:
  class ParserError : std::runtime_error {
    private:
    std::string format_error(int lineno, std::optional<int> columno,
                             std::optional<const char *> why_msg) {
      std::stringstream msg;
      if (LINE_ERR_ONLY){
        msg << lineno;
      }else{
        msg << "Parsing error at line " << lineno;
        if (columno.has_value()) {
          msg << " and column " << columno.value();
        }
        if (why_msg.has_value()) {
          msg << " because: " << why_msg.value();
        }
        msg << "\n";
      }


      if (PRINT_ERR){
        std::cerr << msg.str() << std::endl;
      }
      exit(1);
      return msg.str();
    };

  public:
    ParserError(int lineno)
        : std::runtime_error(
              format_error(lineno, std::nullopt, std::nullopt)){};

    ParserError(int lineno, int columno)
        : std::runtime_error(format_error(lineno, columno, std::nullopt)){};

    ParserError(int lineno, int columno, const char *why)
        : std::runtime_error(format_error(lineno, columno, why)){};
    ParserError(const Token &t, const char *why)
        : std::runtime_error(format_error(t.line, t.startc, why)){};
    ParserError(const Token &t)
        : std::runtime_error(format_error(t.line, t.startc, std::nullopt)){};
  };
private:
  std::queue<Token> token_stream;
  std::vector<ASTNode> ast_nodes;
  bool print_error_flag;

public:
  Parser(std::queue<Token> &&tokens,bool print_error = false) { token_stream = std::move(tokens); PRINT_ERR = print_error; }
  
private:
  template <typename T> T extract_token(Token &t) {
    try {
      return std::get<T>(t.value);
    } catch (std::bad_variant_access &err) {
      std::stringstream ss;
      ss << "Invalid " << magic_enum::enum_name(t.token_t) << " token.\n";
      throw ParserError(t, ss.str().c_str());
    }
  }

  std::optional<Token> next(std::optional<token_type> expected = std::nullopt) {
    if (token_stream.empty()) {
      return std::nullopt;
    }
    Token t = token_stream.front();
    if (expected.has_value()) {
      if (t.token_t != expected.value()) {
        std::stringstream ss;
        ss << "Expected ";
        ss << magic_enum::enum_name(expected.value());
        throw ParserError(t, ss.str().c_str());
      }
    }

    return t;
  }

  Token consume(std::optional<token_type> expected = std::nullopt) {
    auto t = next(expected);
    if (t.has_value()) {
      token_stream.pop();
    }

    if (!t.has_value()) {
      throw std::runtime_error("Unexpected end of lexical tokens!");
    }
    return t.value();
  }

  void consume_operator(std::string op, const char *err_msg) {
    auto next_t = consume(token_type::OPERATOR);
    auto paren = extract_token<std::string>(next_t);
    if (paren != op) {
      throw ParserError(next_t, err_msg);
    }
  }

  node_idx add_node(ASTNode node) {
    auto idx = this->ast_nodes.size();
    this->ast_nodes.push_back(node);
    return idx;
  }

  node_idx parse_type(std::optional<Token> first_token = std::nullopt) {
    auto t = first_token.has_value() ? first_token.value() : consume();
    auto str_val = extract_token<std::string>(t);
    node_idx idx;
    if (t.token_t == token_type::IDENT) {
      idx = add_node(Type<TType::TIdent>(str_val));
    } else if (t.token_t == token_type::KEYWORD) {
      if (str_val == "void") {
        idx = add_node(Type<TType::TVoid>());
      } else if (str_val == "char") {
        idx = add_node(Type<TType::TChar>());
      } else if (str_val == "int") {
        idx = add_node(Type<TType::TInt>());
      } else {
        throw ParserError(t.line, t.startc, "Expected typename.");
      }
    } else {
      throw ParserError(t.line, t.startc,
                        "Expected type or identifier in type declaration");
    }
    auto next_opt = next();
    while (next_opt.has_value() &&
           next_opt.value().token_t == token_type::OPERATOR) {
      if (extract_token<std::string>(next_opt.value()) != "*") {
        return idx;
      }
      consume();
      idx = add_node(Type<TType::TPoint>(idx));
      next_opt = next();
    }
    return idx;
  }

  // Based on https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#bib
  node_idx parse_expression(int min_precedence = 0, bool allow_new = true) {
    auto lookahead = next();
    auto next_t = lookahead.value();
    node_idx lhs;

    if (next_t.token_t == token_type::KEYWORD) {
      // a new expression should not be found in an operator chain
      auto word = extract_token<std::string>(next_t);
      consume();
      if (!allow_new || word != "new") {
        throw ParserError(next_t, "Found illegal keyword in expression");
      }
      node_idx type = parse_type();
      consume_operator("[", "[ expected in 'new' expression");
      node_idx arr_size_expr = parse_expression(0, false);
      consume_operator("]", "] expected in 'new' expression");
      node_idx this_node =
          add_node(Expression<EType::ENew>(type, arr_size_expr));
      return this_node;

    } else if (next_t.token_t == token_type::OPERATOR) {
      auto op = extract_token<std::string>(next_t);
      if (op == "(") {
        // handle parenthesis and unary ops at this level
        consume();
        lhs = parse_expression(0, false);
        consume_operator(")", "Expected ) to close expression");
        // UOP allowed as they have highest precedence
      } else {
        auto uop = UOP.find(op);
        if (uop != UOP.end()) {
          consume();
          int precedence = operator_prec(uop->second);
          node_idx inner = parse_expression(precedence, false);
          lhs = add_node(Expression<EType::EUnOp>(uop->second, inner));
        } else {
          throw ParserError(
              next_t, "Unacceptable operator found in start of expression");
        }
      }
    } else if (next_t.token_t == token_type::IDENT) {
      // Handle array access and function calls, then move on to check
      //  operators.
      auto name = extract_token<std::string>(next_t);
      consume();
      lookahead = next();
      next_t = lookahead.value();
      if (next_t.token_t == token_type::OPERATOR) {
        auto op = extract_token<std::string>(next_t);
        if (op == "(") {
          // Function call
          consume();
          lhs = parse_ecall(name);
        } else if (op == "[") {
          consume();
          lhs = add_node(parse_arr_access(name));
        } else {
          lhs = add_node(Expression<EType::EVar>(name));
        }
      } else {
        lhs = add_node(Expression<EType::EVar>(name));
      }
    } else {
      switch (next_t.token_t) {
      case token_type::CHAR:
        lhs = add_node(
            Expression<EType::EChar>(extract_token<std::string>(next_t)));
        break;
      case token_type::UINT:
        lhs = add_node(Expression<EType::EInt>(extract_token<int>(next_t)));
        break;
      case token_type::STRING:
        lhs = add_node(
            Expression<EType::EString>(extract_token<std::string>(next_t)));
        break;
      default:
        throw ParserError(next_t, "Unknown token type found in expression");
      }
      consume();
    }
    lookahead = next();
    next_t = lookahead.value();
    while (next_t.token_t == token_type::OPERATOR) {
      auto op = extract_token<std::string>(next_t);
      auto prec_ptr = BOP.find(op);
      if (prec_ptr == BOP.end()) {
        break;
      }
      int precedence = operator_prec(prec_ptr->second);
      if (precedence < min_precedence) {
        break;
      }
      consume();
      node_idx rhs = parse_expression(
          precedence == 9 ? precedence : precedence + 1, false);
      lhs = add_node(Expression<EType::EBinOp>(prec_ptr->second, lhs, rhs));
      next_t = next().value();
    }
    return lhs;
  }

  node_idx parse_scope() {
    consume_operator("{", "Unexpected operator in start of statement");
    std::vector<node_idx> inner_statements;
    while (true) {
      auto look_ahead = next();
      auto next_t = look_ahead.value();
      if (next_t.token_t == token_type::OPERATOR) {

        auto paren = extract_token<std::string>(next_t);
        if (paren == "}") {
          consume();
          break;
        } else if (paren != "{") {
          throw ParserError(next_t, "Unexpected operator in statement list");
        }
      }
      inner_statements.push_back(parse_statement());
    }
    node_idx this_node = add_node(Statement<SType::SScope>(inner_statements));
    return this_node;
  }

  node_idx parse_if() {
    consume_operator("(", "Expected ( in start of If");
    node_idx expr = parse_expression();
    consume_operator(")", "Expected ) to close If");
    node_idx if_statement = parse_statement();
    auto look_ahead = next();
    auto next_t = look_ahead.value();
    std::optional<node_idx> else_statement;
    if (next_t.token_t == token_type::KEYWORD) {
      if (extract_token<std::string>(next_t) == "else") {
        consume();
        else_statement = parse_statement();
      }
    }
    node_idx this_node;
    if (else_statement.has_value()) {
      this_node = add_node(
          Statement<SType::SIf>(expr, if_statement, else_statement.value()));
    } else {
      this_node = add_node(Statement<SType::SIf>(expr, if_statement));
    }
    return this_node;
  }

  node_idx parse_while() {
    consume_operator("(", "Expected ( to start While");
    node_idx expr = parse_expression();
    consume_operator(")", "Expected ) to close While");
    node_idx while_statement = parse_statement();
    node_idx this_node =
        add_node(Statement<SType::SWhile>(expr, while_statement));
    return this_node;
  }

  node_idx parse_ecall(std::string var_name) {

    std::vector<node_idx> exprs{};
    bool first_expr = true;

    while (true) {
      if (first_expr) {
        Token t = next().value();
        if (t.token_t == token_type::OPERATOR &&
            extract_token<std::string>(t) == ")") {
          consume();
          break;
        }
        exprs.push_back(parse_expression());
        first_expr = false;
        continue;
      }

      Token t = consume(token_type::OPERATOR);
      auto op = extract_token<std::string>(t);
      if (op == ")") {
        break;
      } else if (op != ",") {
        throw ParserError(t,
                          "Need , in enumeration of function call arguments");
      }
      node_idx e = parse_expression();
      exprs.push_back(e);
    }
    node_idx this_node = add_node(Expression<EType::ECall>(var_name, exprs));

    return this_node;
  }

  Expression<EType::EArrayAccess> parse_arr_access(std::string &arr_name) {
    // Assumes one opening [ has been encountered
    node_idx arr_expr = parse_expression(0, false);
    consume_operator("]", "Need closing ] for lvalue");
    Token t = next().value();
    auto op = extract_token<std::string>(t);
    std::optional<std::string> field_name = std::nullopt;
    if (op == ".") {
      consume();
      t = consume(token_type::IDENT);
      field_name = extract_token<std::string>(t);
    }

    if (field_name.has_value()) {
      return Expression<EType::EArrayAccess>(arr_name, arr_expr,
                                             field_name.value());
    } else {
      return Expression<EType::EArrayAccess>(arr_name, arr_expr);
    }
  }

  node_idx parse_assign(std::optional<Token> first_token = std::nullopt) {
    Token t = first_token.has_value() ? first_token.value()
                                      : consume(token_type::IDENT);
    auto var_name = extract_token<std::string>(t);
    t = consume(token_type::OPERATOR);
    auto op = extract_token<std::string>(t);
    std::optional<Expression<EType::EArrayAccess>> arr_expr;
    std::optional<std::string> field_name;
    node_idx expr_node;
    if (op == "(") {
      expr_node = parse_ecall(var_name);
      node_idx this_node = add_node(Statement<SType::SExpr>(expr_node));
      return this_node;
    } else if (op == "[") {
      // lvalue parsing, we will change op in here
      arr_expr = parse_arr_access(var_name);
      t = consume(token_type::OPERATOR);
      op = extract_token<std::string>(t);
    }

    if (op == "=") {
      expr_node = parse_expression();
    } else if (op == "++" || op == "--") {
      node_idx add_one = add_node(Expression<EType::EInt>(1));

      node_idx target;

      if (arr_expr.has_value()) {
        target = add_node(arr_expr.value());
      } else {
        target = add_node(Expression<EType::EVar>(var_name));
      }

      expr_node = add_node(Expression<EType::EBinOp>(
          (op == "++") ? Operator::BPlus : Operator::BMinus, target, add_one));

    } else {
      throw ParserError(t, "Unexpected operator in assignment");
    }
    if (arr_expr.has_value()) {
      auto arr_node = arr_expr.value();
      if (std::holds_alternative<std::string>(arr_node.tertiary_val)) {
        node_idx this_node = add_node(Statement<SType::SArrayAssign>(
            var_name, std::get<node_idx>(arr_node.secondary_val),
            std::get<std::string>(arr_node.tertiary_val), expr_node));
        return this_node;
      }
      node_idx this_node = add_node(Statement<SType::SArrayAssign>(
          var_name, std::get<node_idx>(arr_node.secondary_val),
          expr_node));
      return this_node;
    }
    node_idx this_node =
        add_node(Statement<SType::SVarAssign>(var_name, expr_node));
    return this_node;
  }

  node_idx parse_varassign() {
    // Ident then Operator means it's an assign, else starts with a type decl
    Token t = consume();
    if (t.token_t == token_type::IDENT) {
      auto n = next().value();
      if (n.token_t == token_type::OPERATOR) {
        auto op = extract_token<std::string>(n);
        if (op != "*") {
          return parse_assign(t);
        }
      }
    }

    node_idx type = parse_type(t);

    t = consume(token_type::IDENT);
    auto var_name = extract_token<std::string>(t);

    consume_operator("=", "Expected = in varassignment");

    node_idx expr = parse_expression();

    node_idx this_node =
        add_node(Statement<SType::SVarDef>(type, var_name, expr));
    return this_node;
  }

  node_idx parse_for() {
    consume_operator("(", "Expected ( to start For");
    // Varassign

    node_idx vardef = parse_varassign();

    consume_operator(";", "Expected ; after varassign in For expression");
    node_idx expr = parse_expression();
    consume_operator(";", "Expected ; after conditional in For expression");
    node_idx assign = parse_assign();
    consume_operator(")", "Expected ) to close For ");
    node_idx statement = parse_statement();

    // transform for into While
    // inner scope
    std::vector<node_idx> loop_statements{statement, assign};
    node_idx loop_scope = add_node(Statement<SType::SScope>(loop_statements));
    // Outer scope and while
    node_idx while_node = add_node(Statement<SType::SWhile>(expr, loop_scope));
    std::vector<node_idx> outer_statements{vardef, while_node};
    node_idx this_node = add_node(Statement<SType::SScope>(outer_statements));
    return this_node;
  }

  node_idx parse_statement() {
    auto next_opt = next();
    auto next_t = next_opt.value();
    if (next_t.token_t == token_type::OPERATOR) {
      return parse_scope();
    } else if (next_t.token_t == token_type::KEYWORD) {
      auto word = extract_token<std::string>(next_t);
      if (word == "if") {
        consume();
        return parse_if();
      } else if (word == "while") {
        consume();
        return parse_while();
      } else if (word == "break") {
        consume();
        consume_operator(";", "Expected ; after break");
        node_idx this_node = add_node(Statement<SType::SBreak>());
        return this_node;
      } else if (word == "return") {
        consume();
        next_opt = next();
        next_t = next_opt.value();
        if (next_t.token_t == token_type::OPERATOR) {
          if (extract_token<std::string>(next_t) == ";") {
            consume();
            node_idx this_node = add_node(Statement<SType::SReturn>());
            return this_node;
          }
        }
        node_idx expr = parse_expression();
        consume_operator(";", "Expected ; at end of return statement");
        node_idx this_node = add_node(Statement<SType::SReturn>(expr));
        return this_node;
      } else if (word == "delete") {
        consume();
        consume_operator("[", "Expected [ after delete");
        consume_operator("]", "Expected ] after delete[");
        next_t = consume(token_type::IDENT);
        auto var_name = extract_token<std::string>(next_t);
        consume_operator(";", "Expected ; after delete expression");

        node_idx this_node = add_node(Statement<SType::SDelete>(var_name));
        return this_node;
      } else if (word == "for") {
        consume();
        return parse_for();
      }
    }
    node_idx assignment = parse_varassign();
    consume_operator(";", "Must have ; to end varassign statement");
    return assignment;
  };

  std::vector<Parameter> parse_params(bool field_not_param = false) {
    std::vector<Parameter> params;
    std::string separator = field_not_param ? ";" : ",";
    std::string closer = field_not_param ? "}" : ")";
    while (true) {
      auto next_opt = next();
      if (next_opt.has_value() &&
          next_opt.value().token_t == token_type::OPERATOR) {
        Token next = next_opt.value();
        auto op = extract_token<std::string>(next);
        if (op != closer) {
          throw ParserError(next, "Unexpected token in parameter parsing");
        } else {
          consume();
          break;
        }
      }

      node_idx type_node = parse_type();

      auto t = consume(token_type::IDENT);
      auto ident = extract_token<std::string>(t);

      params.push_back(Parameter{type_node, ident});

      t = consume(token_type::OPERATOR);
      auto sep = extract_token<std::string>(t);
      if (field_not_param) {
        if (sep != ";")
          throw ParserError(t, "Missing ;");
      } else if (sep == closer) {
        break;
      } else if (sep != separator) {
        throw ParserError(
            t, "Expected separator"); // TODO possible bug, accepts ,)
      }
    }
    return params;
  }

  node_idx parse_extern() {
    node_idx type_node = parse_type();
    auto t = consume(token_type::IDENT);
    auto func_ident = extract_token<std::string>(t);

    t = consume(token_type::OPERATOR);

    auto val = extract_token<std::string>(t);
    std::optional<std::vector<Parameter>> func_params;
    if (val == "(") {
      func_params = parse_params();
      t = consume(token_type::OPERATOR);
      val = extract_token<std::string>(t);
    }

    if (val == ";") {
      node_idx this_node;
      if (func_params.has_value()) {
        this_node = add_node(Global<GType::GFuncDecl>(type_node, func_ident,
                                                      func_params.value()));
      } else {
        // TODO is this really a Var?
        this_node = add_node(Global<GType::GVarDecl>(type_node, func_ident));
      }

      return this_node;
    } else {
      throw ParserError(t, "Extern Decl, expected ';' or '('");
    }
  }

  node_idx parse_struct() {
    auto t = consume(token_type::IDENT);
    auto struct_ident = extract_token<std::string>(t);
    consume_operator("{", "Expected { to open struct");
    auto fields = parse_params(true);

    consume_operator(";", "Expected ; to close struct");
    node_idx this_node = add_node(Global<GType::GStruct>(struct_ident, fields));
    return this_node;
  }

  node_idx parse_function() {
    node_idx type_node = parse_type();
    auto t = consume(token_type::IDENT);
    auto ident = extract_token<std::string>(t);
    t = consume(token_type::OPERATOR);
    auto op = extract_token<std::string>(t);
    if (op == "=") {
      node_idx expr = parse_expression();

      consume_operator(";", "Expected ; after function decl");
      node_idx idx = add_node(Global<GType::GVarDef>(type_node, ident, expr));
      return idx;
    } else if (op != "(") {
      throw ParserError(t, "Expected ( or =");
    }

    auto params = parse_params();
    t = next(token_type::OPERATOR).value();
    if (extract_token<std::string>(t) != "{") {
      throw ParserError(t, "Expected { to define function");
    }

    // As { } are left they should make a SScope
    node_idx scope_node = parse_statement();
    try {
      auto scope_type = std::get<SType>(this->ast_nodes[scope_node].type);
      if (scope_type != SType::SScope) {
        throw ParserError(t, "Expected a scope starting from here");
      }
    } catch (std::bad_variant_access &err) {
      throw ParserError(t, "Expected a scope starting from here");
    }

    node_idx idx =
        add_node(Global<GType::GFuncDef>(type_node, ident, params, scope_node));
    return idx;
  }

  node_idx parse_globals() {
    auto t_opt = next();
    auto t = t_opt.value();
    auto word = extract_token<std::string>(t);

    if (word == "extern") {
      consume();
      return parse_extern();
    } else if (word == "struct") {
      consume();
      return parse_struct();
    } else {
      return parse_function();
    }
  }

public:
  std::vector<node_idx> parse_tokens() {
    std::vector<node_idx> global_nodes;
    while (true) {
      auto t_opt = next();
      if (t_opt.value().token_t == token_type::END_OF_FILE) {
        break;
      }

      global_nodes.push_back(parse_globals());
    }
    return global_nodes;
  }

  std::vector<ASTNode>&& dump_nodes() {
    return std::move(ast_nodes);
  }

  const ASTNode &get_node(node_idx idx) const { return ast_nodes.at(idx); }
};


std::queue<Token> tokenize(std::ifstream &code,bool report);

void print_ast(const Parser &p, const std::vector<node_idx> &global_nodes);
void print_expression(const Parser &p,node_idx e_idx);
void print_type(const Parser &p,node_idx t_idx);

void print_params(const Parser &p, const std::vector<Parameter> &params);

#endif
