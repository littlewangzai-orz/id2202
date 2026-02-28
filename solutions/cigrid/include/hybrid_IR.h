#pragma once

#include "ASM_IR.h"
#include "ast.h"
#include "acc.h"
#include "env.h"

typedef ASTNode* IR_SExpr;

//  typedef struct IR_SExpr {
//      ASTNode* expr;
//  }IR_SExpr;

typedef struct IR_SVarAssign{
    char* sym; 
    IR_SExpr expr;
}IR_SVarAssign;

typedef struct IR_SVarDecl{
    char* sym;
    ASTNode* ty;
}IR_SVarDecl;

typedef struct IRStmt {
    enum { IRSExpr, IRSVarAssign, IRSVarDecl } type;
    union {
        IR_SExpr expr;
        IR_SVarAssign varassign;
        IR_SVarDecl vardecl;
    };
} IR_Stmt;

typedef struct IRBlockEnd { // block end
    enum { IRSReturn, IRSEndBranch, IRSEndJump } endType;
    union {
        IR_SExpr returnExpr; // return statement
        struct {             // switch 
            IR_SExpr expr;
            char* sym_1;
            char* sym_2;
        } IR_SBranch;
        char* IR_Jump;     // IR jump
    };
 }IR_Block_End;

typedef struct IRBlock {
    char* sym;        // basic sym
    IR_Stmt** stmts;     // statements
    int stmtCount;      // the number of statements
    IR_Block_End blockend;
}IR_Block;

typedef struct param{
    ASTNode* ty;
    char* sym;
}Param;

typedef struct IRGlobal {
    char* name;          // global name
    char* returnType;    // return type
    Param* params;       // statment list 
    int paramCount;      
    IR_Block** blocks;    // blocks list
    int blockCount;      
} IR_Global;


typedef struct IRFunc {
    char* name;          // function name
    char* returnType;    // return Type
    Param* params;       // parma list
    int paramCount;      // 
    IR_Block** blocks;    // block list
    int blockCount;      // 
} IR_Func;


IR_Global* convertASTToIR(ASTNode* ast);

IR_Stmt* createIRSVarDecl(char* varName, ASTNode* varType);

IR_Stmt* createIRSVarAssign(char* varName, ASTNode* expr);

IR_Block_End createIRSReturn(ASTNode* expr);

IR_SExpr convertExprToIR(ASTNode* expr);

IR_Block* createIRBlock(char* label);

void addIRStmt(IR_Block* block, IR_Stmt* stmt);

void setBlockEnd(IR_Block* block, IR_Block_End blockEnd);

IR_Global* createIRFunc(char* name, char* returnType);

void addIRBlock(IR_Global* func, IR_Block* block);

void printParam(Param* params, int paramCount);

void printIRBlock(IR_Block* block);

void printIRGlobal(IR_Global* irGlobal);

void printIRSVarAssign(IR_Stmt* stmt);

void inst_select_expr(Environment* env, int* n, Accumulator* acc, Operand reg, IR_SExpr expr);

void inst_select_ir_stmts(Environment* env, int* n, Accumulator* acc, IR_Stmt** stmts, int stmt_count);

void inst_select_ir_block(Environment* env, int* n, Accumulator* acc, IR_Block* block);

void hybrid2ASM(Environment* env, int* n, Accumulator* acc, IR_Global* irGlobal);