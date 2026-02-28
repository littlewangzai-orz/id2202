open Printf
open Ast
open Ir
open Asm_ir
open Spilling

let usage_msg = "append [command] <file>"
let pretty_print = ref false
let line_error =   ref false
let print_asm  =   ref false 
let print_ir  =   ref false 
let file       =   ref ""


(* Annonymous function that binds file to the filename *)
let anon_fun filename =
       file := filename


let speclist =
  [("--pretty-print", Arg.Set (pretty_print), "pretty print AST");
   ("--line-error",   Arg.Set (line_error),   "Show line error");
   ("--ir",           Arg.Set (print_ir),     "pretty print IR");
   ("--asm",          Arg.Set (print_asm),    "pretty print ASM")]

let main =  
  try 
    Arg.parse_argv Sys.argv speclist anon_fun usage_msg;
    
  let ch = 
  try open_in (!file)
  with 
  | Sys_error(_) -> exit 2
  in

  let lexbuf = Lexing.from_channel ch in

  let res =
    try Parser.main Lexer.token lexbuf
  with
  | Invalid_argument(_) ->     
      exit 1
  | Failure(_) ->     
      exit 1
  | Lexer.Error_char ->
    if !line_error then (
      fprintf stderr "%d\n" lexbuf.lex_curr_p.pos_lnum;
      exit 1)
    else
      exit 1
  | Lexer.Error_string ->
    if !line_error then( 
      fprintf stderr "%d\n" lexbuf.lex_curr_p.pos_lnum;
      exit 1)
    else
      exit 1
  | Lexer.Eof ->
    if !line_error then( 
      fprintf stderr "%d\n" lexbuf.lex_curr_p.pos_lnum;
      exit 1)
    else
      exit 1
  | Parser.Error ->
    if !line_error then(
      fprintf stderr "%d\n" lexbuf.lex_curr_p.pos_lnum;
      exit 1)
    else
      exit 1
  in
  close_in ch;

  if !pretty_print then
    Printf.printf "%s\n" (pprint_prog res)
  else 
      if !print_ir then 
      (List.iter (Printf.printf "%s") (List.map pprint_ir_global (cfg_prog res) ))
  else
    if !print_asm then
        let (asm_block, num ) = (inst_select_ir_global [] 0 (cfg_prog res)) in
        (* (Printf.printf "\t\tglobal main"; List.iter (Printf.printf "%s") (List.map (pprint_func) (asm_block)));
        Printf.printf "\n\n\n"; *)
        (Printf.printf "\t\tglobal main"; List.iter (Printf.printf "%s") (List.map (pprint_func) (spilling_func num [] asm_block)))
      else 
        ()


  with
  | Arg.Bad(_) -> exit 1

  
