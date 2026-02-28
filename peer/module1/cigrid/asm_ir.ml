open Printf
open Ir
open Ast


type bitsize =
  | Byte 
  | Word 
  | DWord 
  | QWord


type displacement = int

type scale = int

type reg = int * bitsize

type unop = 
  | Inc
  | Dec
  | Push
  | Pop
  | IMul
  | IDiv
  | Not
  | Neg 
  | Setg
  | Setl
  | Setge
  | Setle
  | Sete
  | Setne


type binop = 
  | Add
  | Sub
  | Cmp
  | Mov
  | And
  | Or
  | Xor

type op =
  | Imm of int
  | Reg of reg
  | TReg of reg * sym
  | Mem of bitsize * reg * reg option * scale * displacement
  | NoOp

type jbinop = 
    | Jl 
    | Jg 
    | Jle 
    | Jge 
    | Je 
    | Jne

type inst =
  | UnOp of unop * op
  | BinOp of binop * op * op
  | Call of sym
  | JBinOp of jbinop * sym * sym
  | Cqo



type blockend =
  | Ret
  | Jmp of sym

type block = 
  | Block of sym * (inst list * blockend)

type func = 
  | Func    of sym * block list
  | FuncDec of sym

let param_reg = function  
  | 0 -> (5,QWord)
  | 1 -> (4,QWord)
  | 2 -> (3,QWord)
  | 3 -> (2,QWord)
  | 4 -> (8,QWord)
  | 5 -> (9,QWord)



let choose_reg (i,b) =
  match (i,b) with
  |(0,QWord)  ->"rax"
  |(1,QWord)  ->"rbx"
  |(2,QWord)  ->"rcx" 
  |(3,QWord)  ->"rdx"
  |(4,QWord)  ->"rsi"
  |(5,QWord)  ->"rdi"
  |(6,QWord)  ->"rbp"
  |(7,QWord)  ->"rsp" 
  |(8,QWord)  ->"r8"
  |(9,QWord)  ->"r9"
  |(10,QWord) ->"r10"
  |(11,QWord) ->"r11"
  |(12,QWord) ->"r12" 
  |(13,QWord) ->"r13"
  |(14,QWord) ->"r14"
  |(15,QWord) ->"r15"
  |(0,DWord)  ->"eax"
  |(1,DWord)  ->"ebx"
  |(2,DWord)  ->"ecx" 
  |(3,DWord)  ->"edx"
  |(4,DWord)  ->"esi"
  |(5,DWord)  ->"edi"
  |(6,DWord)  ->"ebp"
  |(7,DWord)  ->"esp" 
  |(8,DWord)  ->"r8d"
  |(9,DWord)  ->"r9d"
  |(10,DWord) ->"r10d"
  |(11,DWord) ->"r11d"
  |(12,DWord) ->"r12d" 
  |(13,DWord) ->"r13d"
  |(14,DWord) ->"r14d"
  |(15,DWord) ->"r15d"
  |(0,Word)  ->"ax"
  |(1,Word)  ->"bx"
  |(2,Word)  -> "cx"
  |(3,Word)  ->"dx"
  |(4,Word)  ->"si"
  |(5,Word)  ->"di"
  |(6,Word)  ->"bp"
  |(7,Word)  -> "sp"
  |(8,Word)  ->"r8w" 
  |(9,Word)  ->"r9w" 
  |(10,Word) ->"r10w"
  |(11,Word) ->"r11w"
  |(12,Word) ->"r12w"
  |(13,Word) ->"r13w"
  |(14,Word) ->"r14w"
  |(15,Word) ->"r15w"
  |(0,Byte)  ->"al"
  |(1,Byte)  ->"bl"
  |(2,Byte)  ->"cl"
  |(3,Byte)  ->"dl"
  |(4,Byte)  ->"sil"
  |(5,Byte)  ->"dil"
  |(6,Byte)  ->"bpl"
  |(7,Byte)  ->"spl"
  |(8,Byte)  ->"r8b"
  |(9,Byte)  ->"r9b"
  |(10,Byte) ->"r10b" 
  |(11,Byte) ->"r11b" 
  |(12,Byte) ->"r12b" 
  |(13,Byte) ->"r13b" 
  |(14,Byte) ->"r14b"      
  |(15,Byte) ->"r15b"     
  
let pprint_jbinop = function
  | Jl  -> "jl "    
  | Jg  -> "jg "
  | Jle -> "jle" 
  | Jge -> "jge" 
  | Je  -> "je "
  | Jne -> "jne"

let pprint_unop  = function 
    | Inc    -> "inc"
    | Dec    -> "dec"
    | Push   -> "push"
    | Pop    -> "pop"
    | IMul   -> "imul"
    | IDiv   -> "idiv"
    | Not    -> "not"
    | Neg    -> "neg"
    | Setg   -> "setg"
    | Setl   -> "setl"
    | Setge  -> "setge"
    | Setle  -> "setle"
    | Sete   -> "sete"
    | Setne  -> "setne"

let pprint_binop = function 
  | Add     -> "add" 
  | Sub     -> "sub"
  | Cmp     -> "cmp"
  | Mov     -> "mov"
  | And     -> "and"
  | Or      -> "or"
  | Xor     -> "xor"


let pprint_bitsize = function
  | Byte     -> "byte"
  | Word     -> "word"
  | DWord    -> "dword"
  | QWord    -> "qword"


let string_of_sym_ir = function
  | Symbol(str) -> str  

let pprint_displacement d = 
  if d = 1 then
      ""
  else
  sprintf "*%d" d

let pprint_scale s = 
  if s = 0 then
      ""
  else
  sprintf "+%d" s

let pprint_reg (i, b)= choose_reg (i,b)

let pprint_treg(i,b) = sprintf "%d"i

let pprint_opreg = function
  | None                -> ""
  | Some(r)             -> pprint_reg r

let rec pprint_op = function
  | Imm(i)              -> sprintf "%d" i
  | Reg(r)              -> pprint_reg r

  | TReg(r,str)         -> 
     string_of_sym_ir str ^ "_" ^ pprint_treg r  
  | Mem(b, r, rp, s, d) -> pprint_bitsize b ^ "[" ^ pprint_reg r ^ pprint_opreg rp ^  pprint_scale s  ^ pprint_displacement d ^"]"
  | NoOp                -> " "

let pprint_block_end = function
  | Ret                 -> "ret" 
  | Jmp(s)              -> "jmp " ^ string_of_sym_ir s
  
  let  rec pprint_inst = function
  | UnOp(u, o)          -> pprint_unop  u ^ " " ^ pprint_op o
  | BinOp(b, o1 , o2)   -> pprint_binop b ^ " " ^ pprint_op o1 ^ ", " ^ pprint_op o2
  | Call(str)           -> "call $" ^ string_of_sym_ir str 
  | JBinOp(j , s1 , _) ->  pprint_jbinop j ^ " "^ string_of_sym_ir s1   
  | Cqo                 -> "cqo"

and  pprint_instList stml = 
  String.concat "\n\t\t" (List.map pprint_inst stml)

let rec pprint_block = function
  | Block(str,(inl, ble))      -> string_of_sym_ir str ^":" ^"\n\t\t"^  pprint_instList inl  ^"\n\t\t"^  pprint_block_end ble  ^"\n"

and  pprint_blockList bl = 
  String.concat "\n\t\t" (List.map pprint_block bl)


let pprint_func = function
  | FuncDec(str)        -> "\n\t\textern " ^ string_of_sym_ir str ^ "\n"
  | Func(str, bl)             -> "\n\t\tsection .text\n" ^ pprint_blockList bl



let bitsize_of_type = function
  | TyVoid           -> QWord
  | TyInt            -> QWord
  | TyChar           -> QWord
  | TyIdent(_)       -> QWord
  | TyPoint(_)       -> QWord

let make_reg l env =
let (n, ty) = List.assoc l env in
TReg((n, bitsize_of_type ty), l)

let tmp_reg n = TReg((n, QWord), sym_of_string("tmp"))



let rec inst_select_expr env n acc reg = function

(* Variable *)
| ExprVar(x) -> ((BinOp(Mov, reg, make_reg x env)::acc), n)

(* char constant *)
| ExprChar(x) -> ((BinOp(Mov, reg, Imm(Char.code x))::acc), n)

(* Integer constant *)
| ExprInt(v) -> ((BinOp(Mov, reg, Imm(v))::acc), n)

| ExprUnOp(UopNegative, ExprInt(v)) -> ((BinOp(Mov, reg, Imm(-v))::acc), n)


(* BinOp: x = x + 1 or x = 1 + x*)
| ExprBinOp(BopAdd, ExprVar(x), ExprInt(v)) | ExprBinOp(BopAdd, ExprInt(v), ExprVar(x))
when reg = make_reg x env -> (BinOp(Add, reg, Imm(v))::acc, n)

(* BinOp: x = x + y *)
| ExprBinOp(BopAdd, ExprVar(x), ExprVar(y))
when reg = make_reg x env -> (BinOp(Add, reg, make_reg y env)::acc, n)

(* BinOp: x = y + x *)
| ExprBinOp(BopAdd, ExprVar(y), ExprVar(x))
when reg = make_reg x env -> (BinOp(Add, reg, make_reg y env)::acc, n)

(* BinOp: x = 1 - 1*)
| ExprBinOp(BopSub, ExprInt(x), ExprInt(v)) -> (BinOp(Mov, reg, Imm(x))::BinOp(Sub, reg, Imm(v))::acc, n)

(* BinOp: x = x - 1 or x = 1 - x*)
| ExprBinOp(BopSub, ExprVar(x), ExprInt(v)) | ExprBinOp(BopSub, ExprInt(v), ExprVar(x))
when reg = make_reg x env -> (BinOp(Sub, reg, Imm(v))::acc, n)

(* BinOp: x = x - y *)
| ExprBinOp(BopSub, ExprVar(x), ExprVar(y))
when reg = make_reg x env -> (BinOp(Sub, reg, make_reg y env)::acc, n)

(* BinOp: x = y - x *)
| ExprBinOp(BopSub, ExprVar(y), ExprVar(x))
when reg = make_reg x env -> (BinOp(Sub, reg, make_reg y env)::acc, n)


(* BinOp: x = e1 + e2 *)
| ExprBinOp(BopAdd, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, reg, r1)::BinOp(Add, reg, r2)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1


(* BinOp: x = e1 - e2 *)
| ExprBinOp(BopSub, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, reg, r1)::BinOp(Sub, reg, r2)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1



(* BinOp: x = e1 * e2 *)
| ExprBinOp(BopMul, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, (Reg((0,QWord))) , r1)::UnOp(IMul,r2)::BinOp(Mov, reg,(Reg((0,QWord))))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1


(* BinOp: x = e1 / e2 *)
| ExprBinOp(BopDiv, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, (Reg((0,QWord))) , r1)::BinOp(Mov,Reg((3,QWord)), Imm(0))::UnOp(IDiv,r2)::BinOp(Mov, reg,(Reg((0,QWord))))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1


(* BinOp: x = e1 % e2 *)
| ExprBinOp(BopMod, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, (Reg((0,QWord))) , r1)::BinOp(Mov, Reg((3,QWord)), Imm(0))::UnOp(IDiv,r2)::BinOp(Mov, reg,(Reg((3,QWord))))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1


(* BinOp: x = e1 == e2 *)
| ExprBinOp(BopEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::UnOp(Sete, Reg((0,Byte)))::BinOp(Mov, reg, Reg((0,QWord)))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1

(* BinOp: x = e1 != e2 *)
| ExprBinOp(BopNotEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::UnOp(Setne, Reg((0,Byte)))::BinOp(Mov, reg, Reg((0,QWord)))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1


(* BinOp: x = e1 >= e2 *)
| ExprBinOp(BopBiggerOrEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::UnOp(Setge, Reg((0,Byte)))::BinOp(Mov, reg, Reg((0,QWord)))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1

(* BinOp: x = e1 <= e2 *)
| ExprBinOp(BopLessOrEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::UnOp(Setle, Reg((0,Byte)))::BinOp(Mov, reg, Reg((0,QWord)))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1

(* BinOp: x = e1 < e2 *)
| ExprBinOp(BopLess, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::UnOp(Setl, Reg((0,Byte)))::BinOp(Mov, reg, Reg((0,QWord)))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1

(* BinOp: x = e1 > e2 *)
| ExprBinOp(BopBigger, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::UnOp(Setg, Reg((0,Byte)))::BinOp(Mov, reg, Reg((0,QWord)))::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2 e2 in
      inst_select_expr env n2 acc2 r1 e1

(* Call expr *)

| ExprCall(str,el) -> 

  let (acc1, n3) = inst_select_expr_list env n [] 0 el in
  ((List.append acc1 (Call(str)::BinOp(Mov,reg,Reg((0,QWord)))::acc)) , n3)


| _ -> failwith "UNKOWN EXPR"

and
 inst_select_expr_list env n acc reg_num = function
  | x::xs -> 
    let (acc2, n2) = inst_select_expr env n acc (Reg(param_reg(reg_num))) x in
    inst_select_expr_list env (n2) acc2 (reg_num + 1) xs
  | []   -> (acc, n) 

let rec inst_select_ir_stmts env n acc = function
  | IRSVarAssign(x, expr)::xs ->
    let (expr_acc, n2) = inst_select_expr env n [] (make_reg x env) expr in
      inst_select_ir_stmts env n2 (List.rev_append expr_acc acc) xs

  | IRSVarDecl(x,ty)::xs -> 
    inst_select_ir_stmts ((x,(n,ty))::env) (n+1) acc xs

  | IRSExpr(expr)::xs -> 

     let (expr_acc, n2) = inst_select_expr env n [] (Reg((0,QWord))) expr in
      inst_select_ir_stmts env n2 (List.rev_append expr_acc acc) xs

  | [] -> (env, n, List.rev acc)

  | _  -> failwith "UNKOWN STMT"


let rec inst_select_cond_expr env n acc reg dest = function
(* Variable *)
| ExprVar(x) -> ((BinOp(Mov, reg, make_reg x env)::acc), n)

(* char constant *)
| ExprChar(x) -> ((BinOp(Mov, reg, Imm(Char.code x))::acc), n)

(* Integer constant *)
| ExprInt(v) -> (BinOp(Mov, Reg((0,QWord)), Imm(1))::(BinOp(Mov, reg, Imm(v))::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Je,dest,dest)::acc), n)

(* BinOp: x = e1 == e2 *)
| ExprBinOp(BopEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Je,dest,dest)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2  e2  in
      inst_select_expr env n2 acc2 r1 e1 

(* BinOp: x = e1 != e2 *)
| ExprBinOp(BopNotEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Jne,dest,dest)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2  e2  in
      inst_select_expr env n2 acc2 r1  e1 

(* BinOp: x = e1 >= e2 *)
| ExprBinOp(BopBiggerOrEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Jge,dest,dest)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2  e2  in
      inst_select_expr env n2 acc2 r1  e1 

(* BinOp: x = e1 <= e2 *)
| ExprBinOp(BopLessOrEqual, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Jle,dest,dest)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2  e2  in
      inst_select_expr env n2 acc2 r1  e1 

(* BinOp: x = e1 < e2 *)
| ExprBinOp(BopLess, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Jl,dest,dest)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2  e2  in
      inst_select_expr env n2 acc2 r1  e1 

(* BinOp: x = e1 > e2 *)
| ExprBinOp(BopBigger, e1, e2) ->
    let (r1, r2) = (tmp_reg n, tmp_reg (n+1)) in
    let n1 = n + 2 in
    let acc1 = (BinOp(Mov, Reg((0,QWord)), r1)::BinOp(Mov, reg, r2)::BinOp(Cmp, Reg((0,QWord)), reg)::JBinOp(Jg,dest,dest)::acc) in
    let (acc2, n2) = inst_select_expr env n1 acc1 r2  e2  in
      inst_select_expr env n2 acc2 r1  e1 


| _ -> failwith "UNKOWN COND EXPR"


let rec inst_select_ir_block_end env n acc = function
    | IRSReturn(None)       -> (acc, Ret, n)

    | IRSReturn(Some(expr)) -> 
      let (expr_acc, n2) = inst_select_expr env n [] (Reg((0,QWord))) expr in
      ((List.append acc expr_acc), Ret,n2)


    | IRSBranch(expr,s1,s2)    -> 

      let (expr_acc, n2) = inst_select_cond_expr env n [] (Reg((1,QWord))) s1 expr in

      (( List.append acc expr_acc ), Jmp(s2) ,n2)
      
      | IRSJump(s)          -> (acc, Jmp(s), n)
      
let rec inst_select_ir_block l l_next acc_ir_block num env = function
      | IRBlock(str, (stl, ble))::xs ->  
        
        let (envi, n, stmtl1) = (inst_select_ir_stmts env num [] stl) in 
        let (stmtl2, end_of_block,n1) = (inst_select_ir_block_end envi n stmtl1 ble) in
        
        inst_select_ir_block l l_next (Block(str,(stmtl2 , end_of_block))::acc_ir_block) n1 envi xs
        
        | [] -> 
                    
          (List.rev (acc_ir_block) , num)


  let rec inst_select_ir_global acc_ir_global nums= function
    | IRFuncDec(str,(t,tstr))::xs ->   inst_select_ir_global ((FuncDec(str))::acc_ir_global) nums xs
    
    | IRFunc(str,(t,tstr,ir_bl))::xs   -> 
      let (func_block, reg_num) = (inst_select_ir_block str None [] nums [] ir_bl  ) in 
      inst_select_ir_global ((Func(str, func_block ))::acc_ir_global) reg_num xs
    | [] -> (List.rev (acc_ir_global), nums)