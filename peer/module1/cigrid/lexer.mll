{
  open Ast
  open Parser
  exception Error_string 
  exception Error_char
  exception Eof
}


let decimal = '0' | ['1' - '9' ]['0' - '9']*
let hexadecimal   = ['0' - '9' 'a' - 'f' 'A' - 'F']+
let hex1 = "0X"
let hex2 = "0x"

let include = '#'[^ '\n']*

let char_c   = '\''
let string_c = '\"'


let ident = ['_' 'a'-'z' 'A' - 'Z']['_' 'a' - 'z' 'A' - 'Z' '0'- '9']*

let allowChar  = (['a'-'z' 'A' - 'Z' '0'- '9'] | [ ^ '\\'  '\'' '\"' '\n' '\t' '\r' ])

let one_lineComment = "//" [^ '\n']*
let multi_lineCommentStart = "/*"
let multi_lineCommentEnd = "*/"
let whitespace = [' ' '\t']
let newline = '\r' | '\n' | "\r\n"

rule token = parse
   |  whitespace | one_lineComment | include
       { token lexbuf }

   | newline
       { Lexing.new_line lexbuf; token lexbuf }

    (* When matching \' call function char_c_get with a buffer of size 1 and lexbuf *)
   | char_c
     {char_c_get (Buffer.create 1) lexbuf}


    (* When matching the first "" call function string_c_get with a buffer of size 16 and lexbuf *)
   | string_c
     {string_c_get (Buffer.create 16) lexbuf}

    (* When matching the 0x or 0X call function hex_get with a buffer of size 16 and lexbuf *)
   | hex1 | hex2
     {hex_get (Buffer.create 16) lexbuf}

    (* When matching the /* call function multi_LineComment with lexbuf *)
   | multi_lineCommentStart
       {multi_LineComment lexbuf}

    | ident as str 
        {
        match str with
        | "break"  -> Break
        | "extern" -> Extern
        | "new"    -> New
        | "while"  -> While
        | "char"   -> Char
        | "for"    -> For
        | "return" -> Return
        | "delete" -> Delete
        | "if"     -> If
        | "struct" -> Struct
        | "else"   -> Else
        | "int"    -> Int
        | "void"   -> Void
        | s        -> Ident(sym_of_string s)
        }
    
    | decimal as lxm 
        {UInt (int_of_string lxm)}

    | ['=']        
        {Assign}
    | ['~']        
        {Tilde}
    | ['!']        
        {Bang}
    | ['+']           
        {Add}
    | ['-']           
        {Sub}
    | ['*']           
        {Mul}
    | ['/']           
        {Div}
    | ['%']           
        {Mod}
    | ['<']          
        {Less}
    | ['>']        
        {Bigger}
    | ['(']     
        {LeftParen}
    | [')']    
        {RightParen}
    | ['{']      
        {LeftCurl}
    | ['}']     
        {RightCurl}
    | [';']         
        {SemiCol}   
    | ['.']         
        {Dot}
    | [',']         
        {Comma}
    | ['[']         
        {LeftSquare}
    | [']']         
        {RightSquare}
    | "<="   
        {LessOrEqual}
    | ">=" 
        {BiggerOrEqual}
    | "=="         
        {Equal}
    | "!="      
        {NotEqual}
    | "&"         
        {BitAnd}
    | "|"          
        {BitOr}
    | "&&"      
        {LogicAnd}
    |  "||"      
        {LogicOr}
    | "<<"     
        {ShiftLeft}
    | ">>"     
        {ShiftRight}
    | "++"
        {PlusPlus}
    | "--"
        {MinusMinus}
    | eof
       { EOF }

    | _ 
       { raise (Error_char) }

    and multi_LineComment = parse
    (* Return to token when matching */ *)
      | multi_lineCommentEnd 
        { token lexbuf }

    (* Increace lines when matching on newline*)
      | newline
       { Lexing.new_line lexbuf; multi_LineComment lexbuf }
  
      (* Error on matching with /* *)
      | multi_lineCommentStart
        {raise (Error_string)}
      
      | eof
        {raise Eof}
      | _ 
        {multi_LineComment lexbuf}

    and char_c_get  buf = parse

    (* When matching the second \' return a token of the content of the buffer *)
      | char_c 
        {Char_const(Buffer.nth buf 0)}

    (* When matching newline add the charracter to the buffer*)
      | '\\' 'n' 
           {Buffer.add_char buf '\n'; char_c_get buf lexbuf}

      | '\\' 'r' 
           {Buffer.add_char buf '\r'; char_c_get buf lexbuf}

     | '\\' 't' 
           {Buffer.add_char buf '\t'; char_c_get buf lexbuf}

    | '\\' '\'' 
           {Buffer.add_char buf '\''; char_c_get buf lexbuf}

    | '\\' '\"' 
           {Buffer.add_char buf '\"'; char_c_get buf lexbuf}

    | '\\' '\\' 
           {Buffer.add_char buf '\\'; char_c_get buf lexbuf}

    (* We are allowed to have one charachter *)      
    | allowChar 
        {Buffer.add_char buf (Lexing.lexeme_char lexbuf 0); char_c_get buf lexbuf}
    | allowChar+
        {raise (Error_char)}

    and string_c_get buf = parse
      | string_c 
        {String_const(sym_of_string (Buffer.contents buf))}

      | '\\' 'n' 
           {Buffer.add_char buf '\n'; string_c_get buf lexbuf}

      | '\\' 'r' 
           {Buffer.add_char buf '\r'; string_c_get buf lexbuf}

     | '\\' 't' 
           {Buffer.add_char buf '\t'; string_c_get buf lexbuf}

    | '\\' '\'' 
           {Buffer.add_char buf '\''; string_c_get buf lexbuf}

    | '\\' '\"' 
           {Buffer.add_char buf '\"'; string_c_get buf lexbuf}

    | '\\' '\\' 
           {Buffer.add_char buf '\\'; string_c_get buf lexbuf}
        
    | allowChar+
        {Buffer.add_string buf (Lexing.lexeme lexbuf); string_c_get buf lexbuf}
    | _
        {raise(Error_string)}

    and hex_get buf = parse
    | hexadecimal
        {Buffer.add_string buf (Lexing.lexeme lexbuf);
        UInt(Float.to_int (num_of_hexa ((Float.of_int (String.length (Buffer.contents buf)) -. 1.))  (Ast.explode (Buffer.contents buf))))}