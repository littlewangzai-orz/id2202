open Ir
open Asm_ir
open Ast


let spilling_op = function 
  | Imm(i)              -> Imm(i)
  | Reg(r)              -> Reg(r)
  | TReg((i,b),str)     -> 
    let r = (7, QWord) in
    Mem(b, r , None, i*8, 1) 
  | Mem(b, r, rp, s, d) -> Mem(b, r, rp, s, d) 
  | NoOp                -> NoOp

let rec spilling_inst num spilling_acc = function
  | UnOp(u, o)::xs          -> spilling_inst num (UnOp(u, spilling_op o)::spilling_acc) xs

  | BinOp(b, o1 , o2)::xs   -> 

    let (r1, r2) = (spilling_op o1,spilling_op o2) in
    
    (match (r1, r2) with 
    | (Mem(b1, r1, rp1, s1, d1), Mem(b2, r2, rp2, s2, d2)) -> 
      
      (match b with

        | Mov -> spilling_inst num (BinOp(Mov, Mem(b1, r1, rp1, s1, d1),  Reg((10,QWord)))::BinOp(Mov, Reg((10,QWord)),  Mem(b2, r2, rp2, s2, d2))::spilling_acc) xs
        | Add -> spilling_inst num (BinOp(Add, Mem(b1, r1, rp1, s1, d1),  Reg((10,QWord)))::BinOp(Mov, Reg((10,QWord)),  Mem(b2, r2, rp2, s2, d2))::spilling_acc) xs
        | _   -> spilling_inst num (BinOp(Sub, Mem(b1, r1, rp1, s1, d1),  Reg((10,QWord)))::BinOp(Mov, Reg((10,QWord)),  Mem(b2, r2, rp2, s2, d2))::spilling_acc) xs
      )

    | _ -> spilling_inst num (BinOp(b,  r1, r2)::spilling_acc) xs
    )
    
  | Call(str)::xs           -> spilling_inst num (Call(str)::spilling_acc) xs

  | Cqo::xs                  -> spilling_inst num (Cqo::spilling_acc) xs

  | JBinOp(j , s1 , s2)::xs-> spilling_inst num (JBinOp(j , s1 , s2)::spilling_acc) xs

  | [] -> 
    
    List.rev spilling_acc

let modify_first_elem num = function
    | Block(str,(inl, ble)) ->
      Block(str,(BinOp(Sub, Reg((7, QWord)) , Imm(num * 8))::inl, ble))

let modify_last_elem num = function
    | Block(str,(inl, ble)) ->
      Block(str,(List.append inl [BinOp(Add, Reg((7, QWord)) , Imm(num * 8))] , ble))

let drop_first = function
      | x::xs -> xs
      | [] -> []

let rec update_ret num acc = function
      | x::xs ->

        (match x with
        
          | Block(str,(inl, Ret)) -> update_ret num ((modify_last_elem num x)::acc) xs
          | _ -> update_ret num (x::acc) xs
        )

      | [] -> List.rev acc
 
let rec spilling_block num spilling_acc = function
   | Block(str,(inl, ble))::xs     -> spilling_block num (Block(str,(spilling_inst num [] inl, ble))::spilling_acc) xs
   | [] -> 
    let si = List.length spilling_acc in
    if si == 1 then
      let rl = List.rev (spilling_acc) in
      [(modify_last_elem num (modify_first_elem num (List.nth rl 0)))]
    else
      let rl = List.rev (spilling_acc) in
      let first = (modify_first_elem num (List.nth rl 0)) in
      let last = (modify_last_elem num (List.nth rl (si-1))) in
      let drop_f = (drop_first rl) in 
      let drop_l = (drop_first (List.rev drop_f)) in 
      let newest =  (first::(List.rev drop_l)) 
    in 
      List.append (update_ret num [] newest) [last]



let rec spilling_func num spilling_acc = function
    | Func(str, bl)::xs   -> spilling_func num (Func(str, spilling_block num [] bl)::spilling_acc) xs
    | FuncDec(str):: xs   -> spilling_func num (FuncDec(str)::spilling_acc) xs
    | [] -> List.rev (spilling_acc)

    