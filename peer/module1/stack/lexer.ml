open Printf

(* Tokens *)
type token =
  | Token_Push
  | Token_Pop
  | Token_Add
  | Token_Sub
  | Token_Mul
  | Token_Div
  | Token_Show
  | Token_Semi
  | Token_Num of int
  | Token_EOF

let pprint_tokens = function
  | Token_Push    -> "PUSH"
  | Token_Pop     -> "POP"
  | Token_Add     -> "ADD"
  | Token_Sub     -> "SUB"
  | Token_Mul     -> "MUL"
  | Token_Div     -> "DIV"
  | Token_Show    -> "SHOW"
  | Token_Semi    -> "SEMI"
  | Token_Num(x)  -> "NUM " ^ string_of_int x
  | Token_EOF     -> "EOF"

let is_digit  x = x >= '0' && x <= '9'
let is_letter x = (x >= 'a' && x <= 'z' || x >= 'A' && x <= 'Z') 

(* Function to match on condition, return tuple of (string, list) *)
let longest_match condition lst =
  let make_string st l = (String.of_seq(List.to_seq(List.rev st)),l) in
  let rec helper acc = function
    | [] -> make_string acc []
    | x::xs -> if condition x then helper (x::acc) xs else make_string acc (x::xs)
  in helper [] lst

let lexing lst =
  let rec helper acc = function
  | x::xs when is_letter x ->
     let (y,ys) = longest_match is_letter (x::xs) in
     (match y with
      | "push" -> helper (Token_Push::acc)  ys
      | "pop"  -> helper  (Token_Pop::acc)  ys
      | "add"  -> helper  (Token_Add::acc)  ys
      | "sub"  -> helper  (Token_Sub::acc)  ys
      | "mul"  -> helper  (Token_Mul::acc)  ys
      | "div"  -> helper  (Token_Div::acc)  ys
      | "show" -> helper (Token_Show::acc)  ys
      | _      -> helper  (Token_EOF::acc)  ys)

      | ';'::xs  -> helper  (Token_Semi::acc) xs
      |'\r'::xs |' '::xs | '\t'::xs | '\n'::xs  -> helper acc  xs

      | x::xs when is_digit x ->
       let (y,ys) = longest_match is_digit (x::xs) in
       helper (Token_Num(int_of_string y)::acc) ys

      | [] -> List.rev acc
      | x::_ -> printf"%c"x;exit 1
  in
  helper [] lst