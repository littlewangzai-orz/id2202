open Printf

let hex_to_float = function
  | '0'  -> 0.  
  | '1'  -> 1. 
  | '2'  -> 2. 
  | '3'  -> 3. 
  | '4'  -> 4. 
  | '5'  -> 5. 
  | '6'  -> 6. 
  | '7'  -> 7. 
  | '8'  -> 8. 
  | '9'  -> 9. 
  | 'A'  -> 10. 
  | 'B'  -> 11. 
  | 'C'  -> 12. 
  | 'D'  -> 13. 
  | 'E'  -> 14. 
  | 'F'  -> 15. 
  | 'a'  -> 10. 
  | 'b'  -> 11. 
  | 'c'  -> 12. 
  | 'd'  -> 13. 
  | 'e'  -> 14. 
  | 'f'  -> 15. 
  | _    -> 0.

(* Basic convertion from hexa to decimal, len is the length of the hexa string without 0x *)
let rec num_of_hexa len = function
    | []   -> 0.
    | x::xs -> ( hex_to_float x)  *. 16. ** len +. num_of_hexa (len -. 1.) xs 

(* Make string to list of chars *)
let explode s = List.init (String.length s) (String.get s)

type sym = Symbol of string

type uop =
  | UopBang
  | UopTilde
  | UopNegative
  | UopPositive

type bop =
  | BopAdd           
  | BopSub           
  | BopMul           
  | BopDiv           
  | BopMod            
  | BopLess
  | BopBigger
  | BopLessOrEqual
  | BopBiggerOrEqual
  | BopEqual
  | BopNotEqual
  | BopBitAnd
  | BopBitOr
  | BopLogicAnd
  | BopLogicOr
  | BopShiftLeft
  | BopShiftRight

type ty =
  | TyVoid  
  | TyInt  
  | TyChar 
  | TyIdent            of sym
  | TyPoint            of ty
  
type expr =
  | ExprVar            of sym 
  | ExprInt            of int 
  | ExprChar           of char 
  | ExprString         of sym 
  | ExprBinOp          of bop * expr * expr 
  | ExprUnOp           of uop * expr 
  | ExprCall           of sym * expr list 
  | ExprNew            of ty * expr 
  | ExprArrayAccess    of sym * expr * sym option 

type stmt =
  | StmtExpr           of expr 
  | StmtVarDef         of ty * sym * expr 
  | StmtVarAssign      of sym * expr 
  | StmtArrayAssign    of sym * expr * sym option * expr 
  | StmtScope          of stmt list 
  | StmtIf             of expr * stmt * stmt option 
  | StmtWhile          of expr * stmt 
  | StmtBreak         
  | StmtReturn         of expr option 
  | StmtDelete         of sym 


type tyStringList = (ty * sym) list

type global =
  | GlobalFuncDef      of  ty * sym * tyStringList * stmt 
  | GlobalFuncDecl     of  ty * sym * tyStringList
  | GlobalVarDef       of  ty * sym * expr 
  | GlobalVarDecl      of  ty * sym  
  | GlobalStruct       of  sym * tyStringList
  
type prog =
  | Prog               of global list


let string_of_sym = function
  | Symbol(str) -> "\"" ^ str ^ "\"" 

let string_const_of_sym = function
  | Symbol(str) ->  str 

let sym_of_string = function 
  | s -> Symbol(s)

let my_print_chars = function
  | '\n' -> "'\\n'"
  | '\r' -> "'\\r'"
  | '\t' -> "'\\t'"
  | '\'' -> "'\\\''"
  | '\"' -> "'\\\"'"
  | '\\' -> "'\\\\'"
  |   c  -> String.of_seq(List.to_seq('\''::c::'\''::[]))

let chars = function
  | '\n' -> "\\n"
  | '\r' -> "\\r"
  | '\t' -> "\\t"
  | '\'' -> "\\\'"
  | '\"' -> "\\\""
  | '\\' -> "\\\\"
  (* make a string of a char *)
  |   c  -> String.make 1 c 

let rec my_print_string str = 
  match str with
    [] -> ""
  | x::xs -> chars x ^ my_print_string xs
  
let pprint_stringOptional= function
  | None                             -> ""
  | Some(strp)                       -> string_of_sym strp

let pprint_uop = function
  | UopBang                         -> "!"
  | UopTilde                        -> "~"
  | UopNegative                     -> "-"
  | UopPositive                     -> "+"

let pprint_bop = function
  | BopAdd                          -> "+"    
  | BopSub                          -> "-"  
  | BopMul                          -> "*"  
  | BopDiv                          -> "/"      
  | BopMod                          -> "%"      
  | BopLess                         -> "<"      
  | BopBigger                       -> ">"      
  | BopLessOrEqual                  -> "<="   
  | BopBiggerOrEqual                -> ">="     
  | BopEqual                        -> "=="     
  | BopNotEqual                     -> "!="    
  | BopBitAnd                       -> "&"   
  | BopBitOr                        -> "|"      
  | BopLogicAnd                     -> "&&"       
  | BopLogicOr                      -> "||"    
  | BopShiftLeft                    -> "<<"       
  | BopShiftRight                   -> ">>"         

let rec pprint_ty = function
  | TyVoid                          -> "TVoid"    
  | TyInt                           -> "TInt"  
  | TyChar                          -> "TChar"  
  | TyIdent(str)                    -> "TIdent(" ^ string_of_sym str ^ ")"    
  | TyPoint(t)                      -> "TPoint(" ^ pprint_ty t ^ ")"    

let rec pprint_tyStringList tsl =
  String.concat " " (List.map (fun (t, str) -> "(" ^ pprint_ty t ^ ", " ^ string_of_sym str ^ ")") tsl)
        
let rec pprint_expr = function
  | ExprVar(str)                    -> "EVar(" ^ string_of_sym str ^ ")"     
  | ExprInt(i)                      -> sprintf "EInt(%d)" i  
  | ExprChar(c)                     -> "EChar(" ^ my_print_chars c ^")" 
  | ExprString(str)                 -> "EString(\"" ^  my_print_string (explode (string_const_of_sym str)) ^ "\")"     
  | ExprBinOp(op,e1,e2)             -> "EBinOp("  ^ pprint_bop op ^ ", " ^ pprint_expr e1 ^ "," ^ pprint_expr e2 ^ ")"    
  | ExprUnOp(op,e)                  -> "EUnOp("   ^ pprint_uop op ^ ", " ^ pprint_expr e ^ ")"      
  | ExprCall(str,el)                -> "ECall("   ^ string_of_sym str           ^ ", " ^"{" ^ pprint_exprList el ^"}"^ ")"      
  | ExprNew(t,e)                    -> "ENew("    ^ pprint_ty t   ^ ", " ^ pprint_expr e ^ ")"   
  | ExprArrayAccess(str,e,strp)     -> "EArrayAccess("  ^ string_of_sym str           ^ ", " ^ pprint_expr e ^ "," ^ pprint_stringOptional strp ^ ")"    
  
and pprint_exprList el =
  String.concat " " (List.map pprint_expr el)

let pprint_exprOptional= function
  | None                            -> ""
  | Some(e)                         -> pprint_expr e

let rec pprint_stmt = function
  | StmtExpr(e)                     -> "SExpr("   ^ pprint_expr e ^ ")"    
  | StmtVarDef(t,str,e)             -> "SVarDef(" ^ pprint_ty   t ^ ", " ^  string_of_sym str ^ "," ^ pprint_expr e ^ ")"  
  | StmtVarAssign(str,e)            -> "SVarAssign(" ^ string_of_sym str ^ "," ^ pprint_expr e ^ ")"    
  | StmtArrayAssign(str,e1,strp,e2) -> "SArrayAssign(" ^ string_of_sym str ^ "," ^ pprint_expr e1 ^ "," ^ pprint_stringOptional strp ^ "," ^ pprint_expr e2 ^ ")"      
  | StmtScope(stml)                 -> "SScope(" ^"{\n\t\t" ^ pprint_stmtList stml ^"\n\t}" ^ ")"       
  | StmtIf(e,stm,stmp)              -> "SIf(" ^ pprint_expr e ^ ",\n\t" ^ pprint_stmt stm ^ ", " ^ pprint_stmtOptional stmp ^ ")"       
  | StmtWhile(e,stm)                -> "SWhile(" ^ pprint_expr e ^ "," ^ pprint_stmt stm ^ ")"       
  | StmtBreak                       -> "SBreak"    
  | StmtReturn(ep)                  -> "SReturn(" ^ pprint_exprOptional ep ^ ")"      
  | StmtDelete(str)                 -> "SDelete(" ^ string_of_sym str ^ ")"     

and pprint_stmtList stml = 
  String.concat "\n\t\t" (List.map pprint_stmt stml)

and pprint_stmtOptional= function
  | None                            -> ""
  | Some(stmp)                      -> pprint_stmt stmp 

let rec pprint_global = function
  | GlobalFuncDef(t,str,tstr,stm)   -> "GFuncDef(" ^ pprint_ty t ^ "," ^ string_of_sym str ^ "," ^"{" ^ pprint_tyStringList tstr ^"}" ^ ",\n\t" ^ pprint_stmt stm ^ ")"     
  | GlobalFuncDecl(t,str,tstr)      -> "GFuncDecl(" ^ pprint_ty t ^ "," ^ string_of_sym str ^ "," ^"{" ^ pprint_tyStringList tstr ^"}" ^ ")"    
  | GlobalVarDef(t,str,e)           -> "GVarDef(" ^ pprint_ty t ^ "," ^ string_of_sym str ^ "," ^ pprint_expr e ^ ")"    
  | GlobalVarDecl(t,str)            -> "GVarDecl(" ^ pprint_ty t ^ "," ^ string_of_sym str ^ ")"        
  | GlobalStruct(str,tstr)          -> "GStruct(" ^ string_of_sym str ^ "," ^"{" ^ pprint_tyStringList tstr ^"}" ^  ")"    
    
let rec pprint_prog = function
  | Prog(gl)                        -> String.concat " \n" (List.map pprint_global gl)   



