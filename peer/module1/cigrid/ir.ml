open Ast


type ir_block_end = 
  | IRSReturn              of expr option
  | IRSBranch              of expr * sym * sym
  | IRSJump                of sym

type ir_stmt =
  | IRSExpr                 of expr 
  | IRSVarAssign            of sym * expr 
  | IRSVarDecl              of sym * ty 

type ir_block = 
  | IRBlock        of sym * (ir_stmt list * ir_block_end)

type ir_global = 
  | IRFunc    of sym * (ty * tyStringList * ir_block list)
  | IRFuncDec of sym * (ty * tyStringList) 


let pprint_ir_block_end = function
  | IRSReturn(ep)                  -> "IRSReturn(" ^ pprint_exprOptional ep ^ ")"      
  | IRSBranch(e,s1,s2)             -> "IRSBranch(" ^ pprint_expr e ^ ",\n\t\t" ^ string_of_sym s1  ^ ", " ^ string_of_sym s2 ^")"      
  | IRSJump(s)                     -> "IRSJump(" ^ string_of_sym s ^ ")"      

let  rec pprint_ir_stmt = function
  | IRSExpr(e)                     -> "IRSExpr("   ^ pprint_expr e ^ ")"    
  | IRSVarAssign(str,e)            -> "IRSVarAssign(" ^ string_of_sym str ^ "," ^ pprint_expr e ^ ")"    
  | IRSVarDecl(str,t)              -> "IRSVarDecl(" ^ string_of_sym str^ "," ^ pprint_ty   t  ^ ")"  
and  pprint_ir_stmtList stml = 
  String.concat "\n\t\t" (List.map pprint_ir_stmt stml)


let rec pprint_ir_block = function
  | IRBlock(str,(ir_stl, ble))      -> "IRBlock({" ^ string_of_sym str ^ ",\n\t\t"^ pprint_ir_stmtList ir_stl  ^"\n\t\t"^  pprint_ir_block_end ble  ^"\n\t})"

and  pprint_ir_blockList bl = 
  String.concat "\n\t" (List.map pprint_ir_block bl)


let  pprint_ir_global = function
  | IRFunc(str,(t,tstr,ir_bl))   -> "IRFunc(" ^ pprint_ty t ^ "," ^ string_of_sym str ^ "," ^ "{" ^ pprint_tyStringList tstr ^"}" ^ ", {\n\t" ^ pprint_ir_blockList ir_bl ^ "\n})"     
  
  | IRFuncDec(str,(t,tstr))      -> "IRFuncDec(" ^ pprint_ty t ^ "," ^ string_of_sym str ^ "," ^ "{" ^ pprint_tyStringList tstr ^"}"^ ")\n"
  
  | _ -> ""


let rec cfg_stmts l l_next acc_stmts blocks num = function
  | StmtExpr(e)::xs -> cfg_stmts l l_next (IRSExpr(e)::acc_stmts) blocks num xs

  | StmtVarDef(ty,s,e)::xs -> cfg_stmts l l_next (IRSVarAssign(s, e)::IRSVarDecl(s, ty)::acc_stmts) blocks num xs

  | StmtVarAssign(s, e)::xs -> cfg_stmts l l_next (IRSVarAssign(s, e)::acc_stmts) blocks num xs

  | StmtScope(lst)::xs -> cfg_stmts l l_next acc_stmts blocks num (List.append lst xs)

  | StmtWhile(e,stm)::xs   -> 
    
      let l_while  =  sym_of_string("_while_"^string_of_int (num)) in
      let l_true   =  sym_of_string("_while_true_"^string_of_int (num + 1)) in
      let l_false  =  sym_of_string("_while_false_"^string_of_int (num + 2)) in

      let b_before = IRBlock(l, (List.rev acc_stmts, IRSJump(l_while))) in 
      let b_while  = IRBlock(l_while, ([], IRSBranch(e, l_true, l_false))) in

      let blocks3 = cfg_stmts l_true (Some(l_while)) [] (b_while::b_before::blocks) (num + 1) [stm] in

        cfg_stmts l_false l_next [] (blocks3) (num + 2) xs


  | StmtIf(e,t_stmt,None)::xs ->

    let l_true =  sym_of_string("_if_true_"^string_of_int (num)) in

    let l_after = sym_of_string("_after_if_"^string_of_int (num+1)) in

    let b_before = IRBlock(l, (List.rev acc_stmts, IRSBranch(e, l_true, l_after))) in

    let blocks2 = cfg_stmts l_true (Some(l_after)) [] (b_before::blocks) (num + 1) [t_stmt] in

      cfg_stmts l_after l_next [] blocks2 (num + 2) xs


    | StmtIf(e,t_stmt, Some(t_stmtp))::xs ->

      let l_true  =  sym_of_string("_if_true_"^string_of_int (num)) in
      let l_false =  sym_of_string("_if_false_"^string_of_int (num + 1)) in
      let l_after =  sym_of_string("_after_if_else_"^string_of_int (num + 2)) in

      let b_before = IRBlock(l, (List.rev acc_stmts, IRSBranch(e, l_true, l_false))) in

      let blocks2 = cfg_stmts l_true (Some(l_after)) [] (b_before::blocks) (num + 1) [t_stmt] in
      let blocks3 = cfg_stmts l_false (Some(l_after)) [] (blocks2) (num + 2) [t_stmtp] in

        cfg_stmts l_after l_next [] (blocks3) (num + 3) xs

    | StmtReturn(e_op)::_ -> IRBlock(l, (List.rev acc_stmts, IRSReturn(e_op)))::blocks
  
    | [] ->
      let blockend =
      (match l_next with
      | None -> IRSReturn(None)
      | Some l2 -> IRSJump(l2)) in
      let last = IRBlock(l, (List.rev acc_stmts,  blockend))::blocks in
      last


let rec cfg_global acc_globals = function
      |  GlobalFuncDef(t,str,tstr,stm)::xs ->
          let block_list =  cfg_stmts str None [] [] 0 [stm] in
          cfg_global (IRFunc(str,(t,tstr, (List.rev block_list)))::acc_globals) xs

      | GlobalFuncDecl(t,str,tstr)::xs -> 

        cfg_global (IRFuncDec(str,(t,tstr))::acc_globals) xs
          
      |  [] -> List.rev (acc_globals)

let cfg_prog = function
        | Prog(g) -> cfg_global [] g