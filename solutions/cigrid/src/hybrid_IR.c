#include "../include/ASM_IR.h"
#include "../include/ast.h"
#include "../include/hybrid_IR.h"
#include "../include/acc.h"
#include "../include/env.h"
#include <stdio.h>
#include <string.h>

// main function to convert AST to IR
IR_Global* convertASTToIR(ASTNode* ast) {
    
    if (strcmp(ast->type, "g") == 0) {
        // initialize IR_Global
        IR_Global* IR_Global = createIRFunc(ast->children[0]->value,ast->children[1]->value); // function name and return type
        IR_Block* mainBlock = createIRBlock(ast->children[1]->value);        // create main block

        // read the scope
        ASTNode* scope = ast->children[3];
       
        ASTNode* currentScope = scope;

        while (currentScope != NULL) {
            
            for (int i = 0; i < currentScope->child_count; i++) {
                
                ASTNode* stmt = currentScope->children[i];
                // process different types of statements

                if (strcmp(stmt->value, "SVarDef") == 0) {

                    // add variable declaration statement
                    char* sym = stmt->children[0]->children[1]->value;
                    ASTNode* ty = stmt->children[0]->children[0];
                    
                    ASTNode* expr = stmt->children[0]->children[2];

                    addIRStmt(mainBlock, createIRSVarDecl(sym,ty)); //sym ty 
                    // createIRSVarAssign(sym, expr)
                    addIRStmt(mainBlock, createIRSVarAssign(sym, expr)); //sym expr
                }
                else if (strcmp(stmt -> value, "SVarAssign") == 0) {
                    char * sym = stmt -> children[0] -> children[0] -> value;
                    ASTNode* expr = stmt -> children[0] -> children[1];
                    addIRStmt(mainBlock, createIRSVarAssign(sym, expr));
                }
                //process return statement
                else if (strcmp(stmt->value, "SReturn") == 0) {
                    mainBlock->blockend = createIRSReturn(stmt);
                }

                //if meet the stmt_list, then renew the scope
                else if (strcmp(stmt->type, "stmt_list") == 0) {
                    
                    if(currentScope->child_count == 1) 
                        currentScope = currentScope->children[0];
                    else if (currentScope->child_count == 2){
                        currentScope = currentScope->children[1];
                    }
                    // renew scope
                    i = -1; // reassign i£¬the next loop will start from 0
                    break; 
                }
            }

            // if the scope does not have a son-scope, break then;
            if (currentScope == scope || currentScope->child_count == 0) {
                break;
            }
        }

        // add_basic_IR_Block
        addIRBlock(IR_Global, mainBlock);

        return IR_Global;
    }
    return NULL;
}

// create Var Decl statement
IR_Stmt* createIRSVarDecl(char* varName, ASTNode* varType) {
    IR_Stmt* stmt = malloc(sizeof(IR_Stmt));
    stmt->type = IRSVarDecl;
    stmt->vardecl.sym = strdup(varName);
    stmt->vardecl.ty = varType;
    return stmt;
}

// create Var Assign statement
IR_Stmt* createIRSVarAssign(char* varName, ASTNode* expr) {
    IR_Stmt* stmt = malloc(sizeof(IR_Stmt));
    stmt->type = IRSVarAssign;
    stmt->varassign.sym = strdup(varName);
    stmt->varassign.expr = convertExprToIR(expr); // transfer AST expr to IR expr
    return stmt;
}

// create Return end block
IR_Block_End createIRSReturn(ASTNode* expr) {
    IR_Block_End* SReturn = malloc(sizeof(IR_Block_End));
    SReturn->endType = IRSReturn;
    SReturn->returnExpr = expr;
    return *SReturn;
}

// transfer AST expr to IR expr (recursively)
IR_SExpr convertExprToIR(ASTNode* expr) {
    IR_SExpr irExpr;
    irExpr = expr; 
    return irExpr;
}

// create basic block
IR_Block* createIRBlock(char* label) {
    IR_Block* block = malloc(sizeof(IR_Block));
    block->sym = strdup(label);
    block->stmts = NULL; 
    block->stmtCount = 0;
    return block;
}

// add statment to the Block
void addIRStmt(IR_Block* block, IR_Stmt* stmt) {
    block->stmts = realloc(block->stmts, sizeof(IR_Stmt*) * (block->stmtCount + 1));
    block->stmts[block->stmtCount++] = stmt;
}



// set block End
void setBlockEnd(IR_Block* block, IR_Block_End blockEnd) {
    block->blockend = blockEnd;
}


// create IR Function
IR_Global* createIRFunc(char* name, char* returnType) {
    IR_Global* func = malloc(sizeof(IR_Global));
    func->name = strdup(returnType);
    func->returnType = strdup(name);
    func->params = NULL;
    func->paramCount = 0;
    func->blocks = NULL;
    func->blockCount = 0;
    return func;
}

// add IR Basic Block
void addIRBlock(IR_Global* func, IR_Block* block) {
    func->blocks = realloc(func->blocks, sizeof(IR_Block*) * (func->blockCount + 1));
    func->blocks[func->blockCount++] = block;
}

// printParam
void printParam(Param* params, int paramCount) {
    
    for (int i = 0; i < paramCount; i++) {
        printf("%s %s\n", params[i].ty->value, params[i].sym); // suppose ASTNode has value
    }
}

// print IR_Block
void printIRBlock(IR_Block* block) {
    printf("{%s,\n",block->sym);
    for (int i = 0; i < block->stmtCount; i++) {
        IR_Stmt* stmt = block->stmts[i];
        switch (stmt->type) {
            case IRSExpr:
                printf("     Expr: %s\n", stmt->expr->value); // suppose ASTNode has value
                break;
            case IRSVarAssign:
                printIRSVarAssign(stmt);
                break;
            case IRSVarDecl:
                printf("     IRSVarDecl(\"%s\",\"%s\")\n", 
                        stmt->vardecl.sym, stmt->vardecl.ty->value);//ty sym
                break;
            default:
                printf("Unknown statement type\n");
        }
       
    }
    switch (block->blockend.endType) {
        case IRSReturn:
            if (block->blockend.returnExpr != NULL) {
                printf("     IRSReturn(");
                print_ast(block->blockend.returnExpr,0);
                printf("\n");
                
            } else {
                printf("    Return: void\n");
            }
            break;
        case IRSEndBranch:
            printf("Branch: %s ? %s : %s\n", 
                   block->blockend.IR_SBranch.expr->value,
                   block->blockend.IR_SBranch.sym_1, 
                   block->blockend.IR_SBranch.sym_2);
            break;
        case IRSEndJump:
            printf("Jump to: %s\n", block->blockend.IR_Jump);
            break;
        default:
            printf("Unknown block end type\n");
    }
    printf("  })\n");
}

// print IR Global
void printIRGlobal(IR_Global* irGlobal) {
    printf("IRFunc(%s,%s,{", irGlobal->returnType,irGlobal->name);
    // print param
    printParam(irGlobal->params, irGlobal->paramCount);
    printf("},");

    // print basic block
    printf("{\n  IRBlock(");

    for (int i = 0; i < irGlobal->blockCount; i++) {
        printIRBlock(irGlobal->blocks[i]);
    }
    printf("})\n");
}

void printIRSVarAssign(IR_Stmt* stmt){
    printf("     IRSVarAssign(\"");
    printf("%s\",",stmt->varassign.sym); //print sym
    //print ast 
    print_ast(stmt -> varassign.expr, 0);
    printf(")\n");
    
}

void hybrid2ASM(Environment *env, int *n, Accumulator *acc, IR_Global *irGlobal){
    printf("        global %s\n", irGlobal->name);
    printf("        section .text\n");
    
    printf("%s:\n", irGlobal->name);
    for (int i = 0; i < irGlobal->blockCount; i++) {
        IR_Block* block = irGlobal->blocks[i];
        inst_select_ir_block(env, n, acc, block);
    }

}
void inst_select_ir_block(Environment* env, int* n, Accumulator* acc, IR_Block* block) {

    inst_select_ir_stmts(env, n, acc, block->stmts, block->stmtCount);

    // process block end
    if (block->blockend.endType == IRSReturn) {
        
        if (block->blockend.returnExpr != NULL) {
            
            Operand rax_op = make_reg_operand("rax"); // return value
            
            inst_select_expr(env, n, acc, rax_op, block->blockend.returnExpr);//?
        }

        // add block_end_instruction
        Inst ret_inst = create_ret_instruction();
        add_instruction(acc, ret_inst);
    } 
}



void inst_select_ir_stmts(Environment* env, int* n, Accumulator* acc, IR_Stmt** stmts, int stmt_count) {
    for (int i = 0; i < stmt_count; i++) {
        IR_Stmt* stmt = stmts[i];

        if (stmt->type == IRSVarDecl) {
            // variable declaration
            add_variable(env, stmt->vardecl.sym, (*n)++, stmt->vardecl.ty);
            //print_environment(env);
        }

        else if(stmt->type == IRSVarAssign){
            
            if(strcmp(stmt ->varassign.expr->type,"EInt") == 0){

                TReg reg = make_reg(stmt->varassign.sym, env);
                
                Operand treg = make_treg_operand(reg);

                inst_select_expr(env, n, acc, treg, stmt->varassign.expr);
            
            }
            else if(strcmp(stmt->varassign.expr->value,"EBinOp") == 0){

                int sum = 0;
                //traverse_ast(stmt->varassign.expr);
                if (is_only_eint_and_binop(stmt->varassign.expr, &sum)) {
                    char* str = (char*)malloc(10 * sizeof(char));  //dynamically allocate memory
                    sprintf(str, "%d", sum);  // change the sum to string
                    stmt->varassign.expr = create_node("EInt", str, 0);

                }
                

                TReg reg = make_reg(stmt->varassign.sym, env);
                
                Operand treg = make_treg_operand(reg);

                inst_select_expr(env,n,acc, treg,stmt->varassign.expr);
            }
            else if(strcmp(stmt -> varassign.expr -> type, "EVar") == 0){
                TReg reg = make_reg(stmt->varassign.sym, env);
                
                Operand treg = make_treg_operand(reg);

                inst_select_expr(env, n, acc, treg, stmt->varassign.expr);
            }
            
        }
    }
}

void inst_select_expr(Environment* env, int* n, Accumulator* acc, Operand treg, IR_SExpr expr){
    if(strcmp(expr -> type,"EInt") == 0){
        //printf("EInt\n");
        int value = atoi(expr -> value); // get the value of the integer

        // create a mov instruction
        Inst mov_inst = create_binop_instruction(BINOP_MOV, treg, make_imm_operand(value));
            
        //  put the instruction into the accumulator
        add_instruction(acc, mov_inst);
    }
    else if(strcmp(expr -> type,"EVar") == 0){
        // get the register of the variable
        TReg reg = make_reg(expr -> value, env);

        // create a register operand
        Operand reg_op = make_treg_operand(reg);

        // create a mov instruction
        Inst mov_inst = create_binop_instruction(BINOP_MOV, treg, reg_op);

        // put the instruction into the accumulator
        add_instruction(acc, mov_inst);
    }
    // BinOp : x = x + 1 | x = 1 + x;
    else if(strcmp(expr->value,"EBinOp") == 0 
            && (strcmp( expr -> children[0] ->type,"EVar") == 0 
            && (strcmp(expr -> children[2]->type,"EInt") ==0))
            && (strcmp(treg.treg.sym, expr -> children[0] -> value) == 0)) {

             TReg treg1 = make_reg(expr ->children[0] -> value, env);

            int value = atoi(expr -> children[2] -> value); // get the value of the integer
            
            Operand treg1_op = make_treg_operand(treg1);

            Inst add_inst = create_binop_instruction(BINOP_ADD, treg1_op, make_imm_operand(value));

            add_instruction(acc, add_inst);
            
    }
    else if(strcmp(expr->value,"EBinOp") == 0 
            && (strcmp( expr -> children[0] ->type,"EInt") == 0 
            && (strcmp(expr -> children[2]->type,"EVar") ==0))
            && (strcmp(treg.treg.sym, expr -> children[2] -> value) == 0)) {

            TReg treg1 = make_reg(expr ->children[2] -> value, env);// Increment *n after use

            int value = atoi(expr -> children[0] -> value); // get the value of the integer

            Operand treg1_op = make_treg_operand(treg1);

            Inst add_inst = create_binop_instruction(BINOP_ADD, treg, make_imm_operand(value));
            add_instruction(acc, add_inst);
    }
    else if(strcmp(expr->value,"EBinOp") == 0 
            && (strcmp( expr -> children[0] ->type,"EVar") == 0 
            && (strcmp(expr -> children[2]->type,"EVar") ==0)
            && ((strcmp(treg.treg.sym, expr -> children[0] -> value) == 0) || (strcmp(treg.treg.sym, expr -> children[2] -> value) == 0))) )
            {

            TReg treg1 = make_reg(expr -> children[0] -> value, env);
            
            TReg treg2 = make_reg(expr -> children[2] -> value, env);

            Operand treg1_op = make_treg_operand(treg1);
            Operand treg2_op = make_treg_operand(treg2);

            Inst add_inst = create_binop_instruction(BINOP_ADD, treg1_op,treg2_op );

            add_instruction(acc, add_inst);
    }
    else if(strcmp(expr->value, "EBinOp") == 0) {
        // Temporary registers for sub-expressions
        if(strcmp( expr ->children[1]->value, "+") == 0){
            TReg treg1 = tmp_reg((*n)++);  // Increment *n after use
            TReg treg2 = tmp_reg((*n)++);

            Operand treg1_op = make_treg_operand(treg1);
            Operand treg2_op = make_treg_operand(treg2);

            // Recursively process left and right sub-expressions
            inst_select_expr(env, n, acc, treg1_op, expr->children[0]);
            inst_select_expr(env, n, acc, treg2_op, expr->children[2]);

            // Generate the ADD instruction after recursive calls
            Inst mov_inst = create_binop_instruction(BINOP_MOV, treg, treg1_op);
            add_instruction(acc, mov_inst);

            Inst add_inst = create_binop_instruction(BINOP_ADD, treg, treg2_op);
            add_instruction(acc, add_inst);
        }

    }
    else if(strcmp(expr->value,"SReturn") == 0){

        inst_select_expr(env,n,acc,treg,expr->children[0]);
        
    }
    
    
}