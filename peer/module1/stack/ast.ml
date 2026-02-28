(* Grammar *)
type exprs =
  | Push_Expr of int * exprs 
  | Pop_Expr  of exprs
  | Add_Expr  of exprs
  | Sub_Expr  of exprs
  | Mul_Expr  of exprs
  | Div_Expr  of exprs
  | Show_Expr of exprs
  | EOF_Expr


let rec pprint_expr = function
  | Push_Expr(x , e) -> "push(" ^ string_of_int x ^ ", " ^ pprint_expr e ^ ")"
  | Pop_Expr(e)      -> "pop(" ^  pprint_expr e ^ ")"
  | Add_Expr(e)      -> "add(" ^  pprint_expr e ^ ")"
  | Sub_Expr(e)      -> "sub(" ^  pprint_expr e ^ ")"
  | Mul_Expr(e)      -> "mul(" ^  pprint_expr e ^ ")"
  | Div_Expr(e)      -> "div(" ^  pprint_expr e ^ ")"
  | Show_Expr(e)     -> "show("^  pprint_expr e ^")"
  | EOF_Expr         -> "EOF"