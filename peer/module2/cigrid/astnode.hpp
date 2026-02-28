#ifndef ASTNODE_HPP
#define ASTNODE_HPP
#include <map>
#include <string>
#include <variant>
#include <vector>

enum class Operator {
  UNeg,
  UNot,
  UMinus,
  BPlus,
  BMinus,
  BMul,
  BDiv,
  BMod,
  BLt,
  BGt,
  BLeq,
  BGeq,
  BEq,
  BNeq,
  BBitAnd,
  BBitOr,
  BAnd,
  BOr,
  BShLeft,
  BShRight
};

static int operator_prec(Operator op) {
  switch (op) {
  case Operator::BOr:
    return 0;
  case Operator::BAnd:
    return 1;
  case Operator::BBitOr:
    return 2;
  case Operator::BBitAnd:
    return 3;
  case Operator::BEq:
  case Operator::BNeq:
    return 4;
  case Operator::BLt:
  case Operator::BLeq:
  case Operator::BGt:
  case Operator::BGeq:
    return 5;
  case Operator::BShLeft:
  case Operator::BShRight:
    return 6;
  case Operator::BMinus:
  case Operator::BPlus:
    return 7;
  case Operator::BMod:
  case Operator::BMul:
  case Operator::BDiv:
    return 8;
  case Operator::UMinus:
  case Operator::UNot:
  case Operator::UNeg:
    return 9;
  default:
    return 0;
  }
}

static const std::map<std::string, Operator> UOP{
    {"!", Operator::UNot}, {"~", Operator::UNeg}, {"-", Operator::UMinus}};
static const std::map<std::string, Operator> BOP{
    {"+", Operator::BPlus},    {"-", Operator::BMinus},
    {"*", Operator::BMul},     {"/", Operator::BDiv},
    {"%", Operator::BMod},     {"<", Operator::BLt},
    {">", Operator::BGt},      {"<=", Operator::BLeq},
    {">=", Operator::BGeq},    {"==", Operator::BEq},
    {"!=", Operator::BNeq},    {"&", Operator::BBitAnd},
    {"|", Operator::BBitOr},   {"&&", Operator::BAnd},
    {"||", Operator::BOr},     {"<<", Operator::BShLeft},
    {">>", Operator::BShRight}};

enum class TType { TVoid, TInt, TChar, TIdent, TPoint };

enum class EType {
  EVar,
  EInt,
  EChar,
  EString,
  EBinOp,
  EUnOp,
  ECall,
  ENew,
  EArrayAccess
};

enum class SType {
  SExpr,
  SVarDef,
  SVarAssign,
  SArrayAssign,
  SScope,
  SIf,
  SWhile,
  SBreak,
  SReturn,
  SDelete
};

enum class GType { GFuncDef, GFuncDecl, GVarDef, GVarDecl, GStruct };

using node_idx = unsigned long;
struct Parameter {
  node_idx type;
  std::string name;
};

class ASTNode {
public:
  using type_enum = std::variant<TType, EType, SType, GType>;

  using primary_t = std::variant<std::monostate, std::string, int, node_idx,
                                 char, TType, Operator, std::vector<node_idx>>;

  using secondary_t =
      std::variant<std::monostate, node_idx, std::string, std::vector<node_idx>,
                   std::vector<Parameter>>;

  using tertiary_t = std::variant<std::monostate, node_idx, std::string,
                                  std::vector<Parameter>>;

  using quad_t = std::variant<std::monostate, node_idx>;

  type_enum type;
  primary_t primary_val;
  secondary_t secondary_val;
  tertiary_t tertiary_val;
  quad_t quad_val;

  virtual ~ASTNode(){};
  // TODO Maybe std::move parameters
protected:
  ASTNode(type_enum et) : type(et){};
  ASTNode(type_enum et, primary_t p) : type(et), primary_val(p){};
  ASTNode(type_enum et, primary_t p, secondary_t s)
      : type(et), primary_val(p), secondary_val(s){};
  ASTNode(type_enum et, primary_t p, secondary_t s, tertiary_t t)
      : type(et), primary_val(p), secondary_val(s), tertiary_val(t){};
  ASTNode(type_enum et, primary_t p, secondary_t s, tertiary_t t, quad_t q)
      : type(et), primary_val(p), secondary_val(s), tertiary_val(t),
        quad_val(q){};
};

template <TType T> class Type : public ASTNode {

public:
  Type() : ASTNode(T) {
    static_assert(T == TType::TVoid || T == TType::TInt || T == TType::TChar,
                  "Only base types can be constructed without fields");
  };

  Type(primary_t primary) : ASTNode(T, primary) {
    static_assert(T == TType::TIdent || T == TType::TPoint,
                  "Only Ident and Point can be constructed with one field");
  };
};

template <EType E> class Expression : public ASTNode {

public:
  Expression(primary_t primary) : ASTNode(E, primary) {
    static_assert(E == EType::EVar || E == EType::EInt || E == EType::EChar ||
                      E == EType::EString,
                  "Only literals can be constructed with one field");
  };

  Expression(primary_t primary, secondary_t secondary)
      : ASTNode(E, primary, secondary) {
    static_assert(
        E == EType::EUnOp || E == EType::ECall || E == EType::ENew ||
            E == EType::EArrayAccess,
        "Only certain expressions can be constructed with two fields");
  };

  Expression(primary_t primary, secondary_t secondary, tertiary_t tertiary)
      : ASTNode(E, primary, secondary, tertiary) {
    static_assert(E == EType::EBinOp || E == EType::EArrayAccess,
                  "Only Binary ops and optionally array accesses are "
                  "constructed with three fields");
  };
};

template <SType S> class Statement : public ASTNode {

public:
  Statement() : ASTNode(S) {
    static_assert(S == SType::SBreak || S == SType::SReturn,
                  "Only Returns and Breaks are without fields");
  }

  Statement(primary_t primary) : ASTNode(S, primary) {
    static_assert(S == SType::SExpr || S == SType::SReturn ||
                      S == SType::SScope || S == SType::SDelete,
                  "Only Expr, Delete, Scope and Return can be constructed with "
                  "one field");
  };

  Statement(primary_t primary, secondary_t secondary)
      : ASTNode(S, primary, secondary) {
    static_assert(
        S == SType::SVarAssign || S == SType::SIf || S == SType::SWhile,
        "Only Assignment,If and While can be constructed with two fields");
  };

  Statement(primary_t primary, secondary_t secondary, tertiary_t tertiary)
      : ASTNode(S, primary, secondary, tertiary) {
    static_assert(S == SType::SVarDef || S == SType::SIf ||
                      S == SType::SArrayAssign,
                  "Only If, Def and optionally array assign are constructed "
                  "with three fields");
  };

  Statement(primary_t primary, secondary_t secondary, tertiary_t tertiary,
            quad_t quad)
      : ASTNode(S, primary, secondary, tertiary, quad) {
    static_assert(S == SType::SVarDef || S == SType::SIf ||
                      S == SType::SArrayAssign,
                  "Only array assign is constructed with four fields");
  };
};

template <GType G> class Global : public ASTNode {
public:
  Global(primary_t primary, secondary_t secondary)
      : ASTNode(G, primary, secondary) {
    static_assert(G == GType::GVarDecl || G == GType::GStruct,
                  "Only VarDecl and Struct has 2 fields");
  }

  Global(primary_t primary, secondary_t secondary, tertiary_t tertiary)
      : ASTNode(G, primary, secondary, tertiary) {
    static_assert(G == GType::GFuncDecl || G == GType::GVarDef,
                  "Only FuncDecl and VarDef has 3 fields");
  }

  Global(primary_t primary, secondary_t secondary, tertiary_t tertiary,
         quad_t quad)
      : ASTNode(G, primary, secondary, tertiary, quad) {
    static_assert(G == GType::GFuncDef, "Only FuncDef has 4 fields");
  }
};

#endif
