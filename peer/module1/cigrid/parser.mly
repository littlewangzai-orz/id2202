%{
   open Ast
%}

%token <int> UInt
%token <Ast.sym> Ident

%token <char> Char_const
%token <Ast.sym> String_const


%token Assign        "="
%token Add           "+"
%token Sub           "-"
%token Mul           "*"
%token Div           "/"
%token Mod           "%"
%token Less          "<"      
%token Bigger        ">"      
%token LessOrEqual   "<="   
%token BiggerOrEqual ">="     
%token Equal         "=="     
%token NotEqual      "!="    
%token BitAnd        "&"   
%token BitOr         "|"      
%token LogicAnd      "&&"       
%token LogicOr       "||"    
%token ShiftLeft     "<<"       
%token ShiftRight    ">>"     
%token LeftParen     "("
%token RightParen    ")"
%token LeftCurl      "{"
%token RightCurl     "}"
%token SemiCol       ";"

%token Tilde         "~"
%token Bang          "!"

%token PlusPlus      "++"
%token MinusMinus    "--"
%token Dot           "."
%token Comma         ","
%token LeftSquare    "["
%token RightSquare   "]"


%token Break
%token Extern
%token New
%token While
%token Char
%token For
%token Return
%token Delete
%token If
%token Struct
%token Else
%token Int
%token Void

%token EOF
  
%left "||"  
%left "&&"  
%left "|"  
%left "&"  
%left "==" "!=" 
%left "<" ">" "<=" ">="
%left "<<" ">>"
%left "+" "-" 
%left "*" "/" "%" 
%nonassoc UBang UTilde UNegative


%start main
%type <Ast.prog> main

%%

main :
   | p = prog EOF
      {p}

prog :
   | g = global_list
      {Prog(g)}

global_list :
   | {[]}
   | g1 = global
      {[g1]}
   | g1 = global gs = global_list
      {g1::gs}

global :
  |  ret_type = ty id = Ident LeftParen param_list = params RightParen body = stmt  
      {GlobalFuncDef(ret_type, id, param_list, body)}
  | Extern ret_type = ty id = Ident LeftParen param_list = params RightParen SemiCol
      {GlobalFuncDecl(ret_type,id,param_list)}
  | Extern ret_type = ty  id = Ident SemiCol  
      {GlobalVarDecl(ret_type,id)}
  | ret_type = ty id = Ident Assign e = expr SemiCol 
      {GlobalVarDef(ret_type, id, e)}
  | Struct id = Ident LeftCurl param_list = body_list RightCurl SemiCol
      {GlobalStruct(id,param_list)}

body_list : 
   | {[]}
   | t = ty id = Ident
      {[(t,id)]}
   | t = ty id = Ident SemiCol rst = body_list
      {(t,id)::rst}

params :
   | {[]}
   | t = ty id = Ident
      {[(t,id)]}
   | t = ty id = Ident Comma rst = params
      {(t,id)::rst}


stmt:
  | v = varassign SemiCol
    {v}
  | LeftCurl sl = stmt_list RightCurl
    {StmtScope(sl)}

  | If LeftParen cond = expr RightParen then_branch = stmt Else else_branch = stmt
    { StmtIf(cond, then_branch, Some(else_branch)) }
  | If LeftParen cond = expr RightParen then_branch = stmt
    { StmtIf(cond, then_branch, None) }
  

  | While LeftParen e = expr RightParen s = stmt
      {StmtWhile(e,s)}  
  
  |   Break SemiCol
      {StmtBreak} 

  |   Return op = expr SemiCol
      {StmtReturn(Some(op))}  

   |   Return SemiCol
      {StmtReturn(None)}     
  |   Delete LeftSquare RightSquare id = Ident SemiCol
      {StmtDelete(id)}


| For LeftParen va = varassign SemiCol e = expr SemiCol a = assignment RightParen s = stmt
    {
      StmtScope([
        va;                             (* Variable assignment (initialization) eg i = 0*)
        StmtWhile(
          e,                             (* Condition: the while loop condition eg i < 10 *)
          StmtScope(List.append [s] [a])   (* The body of the for-loop followed by the increment of (initialization) *)
        )
      ])
    }


varassign :
   | t = ty id = Ident Assign e = expr
      {StmtVarDef(t,id,e)}
   |  s = assignment
      {s}

assignment :
   | id = Ident LeftParen expr_list = list_expr RightParen
      {StmtExpr(ExprCall(id, expr_list))} 

   | l = Ident Assign e = expr
      {StmtVarAssign(l,e)}

   | id = Ident LeftSquare e1 = expr RightSquare option_string = AccessOption Assign e2 = expr
      {StmtArrayAssign(id,e1,option_string,e2)}

   | l = Ident PlusPlus
      {StmtVarAssign(l,(ExprBinOp(BopAdd,ExprVar(l),ExprInt(1))))}

   | id = Ident LeftSquare e = expr RightSquare option_string = AccessOption PlusPlus
      {StmtArrayAssign(id,e,option_string,ExprBinOp(BopAdd,ExprArrayAccess(id,e,option_string),ExprInt(1)))}

   | l = Ident MinusMinus
      {StmtVarAssign(l,(ExprBinOp(BopSub,ExprVar(l),ExprInt(1))))}

   | id = Ident LeftSquare e = expr RightSquare option_string = AccessOption MinusMinus
      {StmtArrayAssign(id,e,option_string,ExprBinOp(BopSub,ExprArrayAccess(id,e,option_string),ExprInt(1)))}

stmt_list :
   | {[]}
   | s = stmt sl = stmt_list
    {s::sl}

expr :
  |   id = Ident
      {ExprVar(id)}     
  |   n = UInt
      {ExprInt(n)}    
  |   c = Char_const
      {ExprChar(c)}    
  |   str = String_const
      {ExprString(str)}    

  |   e1 = expr "||" e2 = expr
      {ExprBinOp(BopLogicOr,e1,e2)}   

  |   e1 = expr "&&" e2 = expr
      {ExprBinOp(BopLogicAnd,e1,e2)}  


   |   e1 = expr "|" e2 = expr
      {ExprBinOp(BopBitOr,e1,e2)}  

   |   e1 = expr "&" e2 = expr
      {ExprBinOp(BopBitAnd,e1,e2)}  


   |   e1 = expr "==" e2 = expr
      {ExprBinOp(BopEqual,e1,e2)}  


   |   e1 = expr "!=" e2 = expr
      {ExprBinOp(BopNotEqual,e1,e2)}  


   |   e1 = expr "<" e2 = expr
      {ExprBinOp(BopLess,e1,e2)}  

   |   e1 = expr ">" e2 = expr
      {ExprBinOp(BopBigger,e1,e2)}  


   |   e1 = expr "<=" e2 = expr
      {ExprBinOp(BopLessOrEqual,e1,e2)}  


   |   e1 = expr ">=" e2 = expr
      {ExprBinOp(BopBiggerOrEqual,e1,e2)}  


   |   e1 = expr "<<" e2 = expr
      {ExprBinOp(BopShiftLeft,e1,e2)}  

   |   e1 = expr ">>" e2 = expr
      {ExprBinOp(BopShiftRight,e1,e2)}  

   |   e1 = expr "+" e2 = expr
      {ExprBinOp(BopAdd,e1,e2)}  

   |   e1 = expr "-" e2 = expr
      {ExprBinOp(BopSub,e1,e2)}  

  |   e1 = expr "*" e2 = expr
      {ExprBinOp(BopMul,e1,e2)} 

  |   e1 = expr "/" e2 = expr
      {ExprBinOp(BopDiv,e1,e2)} 

  |   e1 = expr "%" e2 = expr
      {ExprBinOp(BopMod,e1,e2)}  

  |   "!" e = expr %prec UBang
      {ExprUnOp(UopBang,e)} 

   |   "~" e = expr %prec UTilde
      {ExprUnOp(UopTilde,e)}  

   |   "-" e = expr  %prec UNegative
      {ExprUnOp(UopNegative,e)} 

  |   id = Ident LeftParen expr_list = list_expr RightParen
      {ExprCall(id, expr_list)}    
  |   New ret_type = ty LeftSquare e = expr RightSquare
      {ExprNew(ret_type,e)}
  |   id = Ident   LeftSquare e = expr RightSquare option_string = AccessOption
      {ExprArrayAccess(id,e,option_string)}     
  |   LeftParen e = expr RightParen
      {e}  

list_expr :
   | {[]}
   | e = expr
      {[e]}
   | e = expr Comma el = list_expr
      {e::el}

AccessOption :
   | {None}
   | Dot id = Ident
      {Some(id)}

ty :
  |   Void
      {TyVoid} 
  |   Int
      {TyInt}  
  |   Char
      {TyChar} 
  |   id = Ident
      {TyIdent(id)}
  |   t = ty "*"
      {TyPoint(t)}