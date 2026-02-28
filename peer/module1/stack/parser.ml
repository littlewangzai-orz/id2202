open Printf
open Lexer
open Ast


let rec parse_expr  = function
  (* 'push' expression *)
  | Token_Push::Token_Num(x)::Token_Semi::next_tokens ->
    let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Push_Expr(x,expr))

  (* 'pop' expression *)
  | Token_Pop::Token_Semi::next_tokens ->
    let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Pop_Expr(expr))

  (* 'add' expression *)
  | Token_Add::Token_Semi::next_tokens ->
        let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Add_Expr(expr))
  
  (* 'sub' expression *)
  | Token_Sub::Token_Semi::next_tokens ->
        let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Sub_Expr(expr))

  (* 'mul' expression *)  
  | Token_Mul::Token_Semi::next_tokens ->
        let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Mul_Expr(expr))

  (* 'div' expression *)
  | Token_Div::Token_Semi::next_tokens ->
        let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Div_Expr(expr))

  (* 'show' expression *)
  | Token_Show::Token_Semi::next_tokens ->
        let (tokens2, expr) = parse_expr next_tokens in
    (tokens2 ,Show_Expr(expr))

  (* 'EOF' expression *)
  | Token_EOF::next_tokens ->
    (next_tokens ,EOF_Expr)
  |_ -> exit 1

(* Parse function, return a list of the ASTs *)
let parse tokens =
  let rec helper acc tokens =
    match tokens with
    | [] -> List.rev acc
    | _ ->
        let (tokens2, expr) = parse_expr tokens in
        helper (expr :: acc) tokens2
  in
  helper [] tokens