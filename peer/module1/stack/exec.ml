open Printf
open Lexer
open Parser
open Ast

let push x stack =
  List.append stack [x]


let pop stack =
    List.init (List.length stack - 1) (List.nth stack)

     

let add stack =
    let (x1,x2) = (List.nth stack (List.length stack - 2) , List.nth stack (List.length stack - 1)) in
    List.append stack [x2 + x1]

     

let sub stack =
    let (x1,x2) = (List.nth stack (List.length stack - 2) , List.nth stack (List.length stack - 1)) in
      List.append stack [x2 - x1]

     

  let mul stack =
      let (x1,x2) = (List.nth stack (List.length stack - 2) , List.nth stack (List.length stack - 1)) in
      List.append stack [x2 * x1]

       

let div stack =
    let (x1,x2) = (List.nth stack (List.length stack - 2) , List.nth stack (List.length stack - 1)) in
    List.append stack [x2 / x1]

     
  
let show stack =  
  match List.rev stack with
  | [] -> print_endline "stack is empty"; stack
  | x::_ -> Printf.printf "%d\n" x; stack
    

(* Stack operations, operate on the returned list   *)
let rec exec stack expr = 
  match expr with
  | Push_Expr(x, e) -> exec (push x stack) e
  | Pop_Expr(e)     -> exec (pop stack) e
  | Add_Expr(e)     -> exec (add stack) e
  | Sub_Expr(e)     -> exec (sub stack) e
  | Mul_Expr(e)     -> exec (mul stack) e
  | Div_Expr(e)     -> exec (div stack) e
  | Show_Expr(e)    -> exec (show stack) e
  | EOF_Expr        -> ()