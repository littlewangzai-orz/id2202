(* To parse command-line arguments, checkout OCamls `Arg` module
   (https://ocaml.org/manual/5.1/api/Arg.html). To control the exit code on
   errors you can use `Arg.parse_argv` instead of `Arg.parse` *)

open Printf
open Lexer
open Parser
open Ast
open Exec


(* Function with no arguments, reads chars from stdin and append to acc list, add EOF at the end*)
let read_chars () =
  let rec reading lst = 
    try
      let chr = input_char  stdin in
      reading (chr :: lst) 
    with
    | End_of_file -> (List.append ['F';'O';'E'] lst)
  in
  List.rev (reading [])

let main =
   let k  = read_chars() in
   List.map (exec []) (parse (lexing k))



   