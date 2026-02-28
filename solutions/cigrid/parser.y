%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./include/ast.h"
extern char *yytext; 
ASTNode* program = NULL; 
extern int error_flag;
int yyerror(const char *msg);
int yylex();
%}

%debug

%union {
    char *str;  
    struct ASTNode* node;
}

%token <str> T_VOID T_INT T_CHAR T_IDENTIFIER 

%token T_EXTERN
%token T_STAR

%token <str> T_DEC_CONSTANT  T_CHAR_CONSTANT

%token T_IF T_ELSE T_WHILE T_RETURN T_BREAK
%token T_ASSIGN T_SEMICOLON T_COMMA 

%token T_NOT T_BITWISE_NOT T_MINUS 

%token T_PLUS T_DIVIDE T_MODULO

%token T_LESS T_GREATER T_LESS_EQUAL T_GREATER_EQUAL

%token T_EQUAL T_NOT_EQUAL

%token T_AND T_OR T_BITWISE_AND T_BITWISE_OR

%token T_LEFT_SHIFT T_RIGHT_SHIFT

%token  T_DOT T_LPAREN T_RPAREN T_LBRACKET T_RBRACKET T_LBRACE T_RBRACE

%right T_ASSIGN           

%left T_LOGICAL_OR          

%left T_LOGICAL_AND        

%left T_BITWISE_OR          

%left T_BITWISE_AND         

%left T_EQUAL T_NOT_EQUAL 

%left T_LESS_THAN T_GREATER_THAN T_LESS_EQUAL T_GREATER_EQUAL 

%left  T_MINUS  T_PLUS     

%left T_MODULO T_DIVIDE T_STAR 
 

 

%right T_NOT T_BITWISE_NOT 

%right UMINUS

%left T_INCREMENT T_DECREMENT 

%left T_LPAREN T_RPAREN     

%type <node> program global params stmt  expr  varassign assign ty stmt_list global_list
%type <str>  unop lvalue 
    

%%

program:
    {}
    |
    global_list {
        program = $1; 
    }
    ;

global_list:
    global {
        $$ = create_node("p", "globals", 1, $1); 
    }
    | global_list global {
        $$ = create_node("p", "globals", 2, $1, $2); 
    }
    ;

global:
   ty T_IDENTIFIER T_LPAREN params T_RPAREN stmt {
        
        $$ = create_node("g", "GFuncDef", 4, 
            $1,                              
            create_node("TIdent", $2, 0), 
            $4,                              
            $6                              
        );
        
        
       
    }
    | T_EXTERN ty T_IDENTIFIER T_LPAREN params T_RPAREN T_SEMICOLON {

        $$ = create_node("g", "GFuncDecl", 3, 
            $2,                              
            create_node("TIdent", $3, 0), 
            $5                            
        );
        
       
    }
    ;
params:
    { 
        $$ = create_node("l","",0); 
        
    }
    | ty {
        $$ = create_node("l", "params_o", 1, 
            $1                              
        );
    }
    | ty T_IDENTIFIER { 
        $$ = create_node("l", "params_n", 2, 
            $1,                              
            create_node("TIdent", $2, 0)
        );
    }
    | params T_COMMA params {
        $$ = create_node("l", "params_l", 2, 
            $1,                              
            $3
        );
        
    }
    ;
stmt_list:
    {
        $$ = create_node("stmt_list", "", 0); 
    }
    | stmt stmt_list {
        $$ = create_node("stmt_list", "", 2, $1, $2); 
    }
    ;
stmt:
    {
        $$ = create_node("NULL","",0);
    }
    |
    varassign T_SEMICOLON {
        if(!strcmp($1->value,"ECall")){
            $$ = create_node("s","SExpr",1,$1);
        }
        else if (!strcmp($1->value,"varassign")){
            $$ = create_node("s","SVarAssign",1,$1);
        }
        else if (!strcmp($1->value,"tyassign")){
            $$ = create_node("s","SVarDef",1,$1);
        }
    }
    | T_LBRACE stmt_list T_RBRACE {
        $$ = create_node("s", "SScope", 1, $2 );
        //printf("SScope");
    }
    | T_IF T_LPAREN expr T_RPAREN stmt {
        $$ = create_node("s", "SIf", 2, $3, $5);
    }
    | T_IF T_LPAREN expr T_RPAREN stmt T_ELSE stmt {
        $$ = create_node("s", "SIf", 3, $3, $5, $7);
    }
    | T_WHILE T_LPAREN expr T_RPAREN stmt {
       $$ = create_node("s", "SWhile", 2, $3, $5);
    }
    | T_BREAK T_SEMICOLON {
         $$ = create_node("s", "SBreak", 0);
    }
    | T_RETURN expr T_SEMICOLON {
        $$ = create_node("s", "SReturn", 1, $2 ); 
    }
    | T_RETURN T_SEMICOLON {
        $$ = create_node("s", "SReturn",0 );
    }
    ;
varassign:
    ty T_IDENTIFIER T_ASSIGN expr {
         $$ = create_node("tyassign","tyassign",3,$1,create_node("",$2,0),$4);
    }
    | assign {
        $$ = $1;
    }
    ;
lvalue:
    T_IDENTIFIER {
        $$ = create_node("lvalue",$1,0);
    }
    ;
assign:
    T_IDENTIFIER T_LPAREN expr T_RPAREN {
        $$ = create_node("e","ECall",2,create_node("",$1,0),$3);

    }
    | lvalue T_ASSIGN expr {
        $$ = create_node("varassign","varassign",2,$1,$3);
    }
    | lvalue T_INCREMENT {

    }
    | lvalue T_DECREMENT {

    }
    ;


ty:
    T_VOID { $$ = create_node("T","TVoid",0); 
           }
    | T_INT {$$ = create_node("T","TInt",0); }
    | T_CHAR { $$ = create_node("T","TChar",0); }
    | T_IDENTIFIER { }
    | ty T_STAR {}
    ;

expr:
    { $$ = create_node("","",0);}
    | expr T_COMMA expr{
        $$ = create_node("commaexpr","",2, $1, $3);
    }
    |
    T_IDENTIFIER {
        $$ = create_node("EVar", $1 ,0);
    }
    | T_DEC_CONSTANT {
        $$ = create_node("EInt", $1 ,0);
        
    }
    | T_CHAR_CONSTANT {
        $$ = create_node("EChar", $1 ,0);
    }
    | expr T_LOGICAL_OR expr  {
        if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","||",0), $3);
         
    }
    | expr T_LOGICAL_AND expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","&&",0), $3);
         
    }
    | expr T_BITWISE_OR expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","|",0), $3);
         
    }
    | expr T_BITWISE_AND expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","&",0), $3);
         
    }
    | expr T_EQUAL expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","==",0), $3);
         
    }
    | expr T_NOT_EQUAL expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","!=",0), $3);
         
    }
    | expr T_LESS_THAN expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","<",0), $3);
         
    }
    | expr T_GREATER_THAN expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop",">",0), $3);
         
    }
    | expr T_LESS_EQUAL expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","<=",0), $3);
         
    }
    | expr T_GREATER_EQUAL expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop",">=",0), $3);
       
    }
    // left shift and right shift not finished yet
    | expr T_MINUS expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","-",0), $3);
         
    }
    | expr T_PLUS expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","+",0), $3);
       
    }
    | expr T_MODULO expr  {
        if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
        else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","%",0), $3);
        
    }
    | expr T_STAR expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","*",0), $3);
         //printf("e:%s\n",$$->value);
         //printf("expr:%s,binop:%s,expr:%s\n",$1->value,$2->value,$3->value);
    }
    | expr T_DIVIDE expr  {
         if(strcmp($1->value,"") == 0 || strcmp($3->value,"") == 0)
            error_flag = 1;
         else $$ = create_node("e", "EBinOp",3 ,$1, create_node("bop","/",0), $3);;
         //printf("e:%s\n",$$->value);
         //printf("expr:%s,binop:%s,expr:%s\n",$1->value,$2->value,$3->value);
    }
    | unop expr %prec UMINUS {
        //$$ = (char*)malloc(strlen($1) + strlen($2) + 4);
        //sprintf($$, "(%s %s)", $1, $2);
        //printf("Parsed unary operation: %s\n", $$); 
    }
    | T_IDENTIFIER T_LPAREN expr T_RPAREN {
        //ECall
        $$ = create_node("e","ECall",2,create_node(" ",$1,0),$3);
        //printf("e:%s\n",$$->value);
        //printf("expr:%s,Iden:%s\n",$1,$3->value);
    }
    | expr T_LBRACKET expr T_RBRACKET T_DOT T_IDENTIFIER {
        //$$ = (char*)malloc(strlen($1) + strlen($3) + strlen($6) + 10);
        //sprintf($$, "%s[%s].%s", $1, $3, $6);
        //printf("Parsed array access with field: %s\n", $$);
    }
    | T_LPAREN expr T_RPAREN {
        $$ = $2;
    }
    ;
unop:
    T_NOT {
        $$ = create_node("bop","!",0);
        //printf("Parsed unary operator: %s\n", $$);
    }
    | T_BITWISE_NOT {
        $$ = create_node("bop","~",0);
        //printf("Parsed unary operator: %s\n", $$);
    }
    | UMINUS {
        $$ = create_node("bop","-",0);
        //printf("Parsed unary operator: %s\n", $$);
    }
    ;
%%
int yyerror(const char *msg) {
    //fprintf(stderr, "Syntax error: %s\n", msg);
    //error_flag = 1; 
    return 0;
}
