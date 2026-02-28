%{
#include <stdio.h>
#include "parser.tab.h"
enum Tokens {
    //T_BREAK = 256,
    //T_EXTERN,
    T_NEW,
    //T_WHILE,
    //T_CHAR,
    T_FOR,
    //T_RETURN,
    T_DELETE,
    //T_IF,
    T_STRUCT,
    //T_ELSE,
    //T_INT,
    //T_VOID,
    //T_IDENTIFIER,
    T_HEX_CONSTANT,
    //T_DEC_CONSTANT,
    T_ERROR,
    //T_NOT,
    //T_BITWISE_NOT,
    //T_MINUS,
    //T_ASSIGN,
    //T_PLUS,
    //T_MULTIPLY,
    //T_DIVIDE,
    //T_MODULO,
    //T_LEFT_SHIFT,
    //T_RIGHT_SHIFT,
    //T_LESS_THAN,
    //T_GREATER_THAN,
    //T_LESS_EQUAL,
    //T_GREATER_EQUAL,
    //T_EQUAL,
    //T_NOT_EQUAL,
    //T_BITWISE_AND,
    //T_BITWISE_OR,
    //T_LOGICAL_AND,
    //T_LOGICAL_OR,
    //T_semicolon,
    //T_CHAR_CONSTANT
};
int comment_level = 0; // comment count
int error_flag = 0;    // error flag
%}

%x COMMENT
%x DIRECTIVE

Identifier         [_a-zA-Z][_a-zA-Z0-9]* 
DIGIT              [0-9]
NONZERO_DIGIT      [1-9]
HEX_PREFIX         0[xX]
HEX_DIGIT          [0-9a-fA-F]

chrlit          '([^"'\\\n \t]|\\.)'

untermchrlit    ('\s')

decimal 0|[1-9][0-9]*

%%

"break" {return T_BREAK;}
"extern" {return T_EXTERN;}
"new" {return T_NEW;}
"while" {return T_WHILE;}
"char" {return T_CHAR;}
"for" {return T_FOR;}
"return" {return T_RETURN;}
"delete" {return T_DELETE;}
"if" {return T_IF;}
"struct" {return T_STRUCT;}
"else" {return T_ELSE;}
"int" {return T_INT;}
"void" {//printf("TVOID\n");
        return T_VOID;}
"(" { //printf("(\n");
      return T_LPAREN; }
")" { //printf(")\n");
      return T_RPAREN; }
"[" { return T_LBRACKET; }
"]" { return T_RBRACKET; }
"," { return T_COMMA; }
"." { return T_DOT; }
"!" {return T_NOT;}
"~" {return T_BITWISE_NOT;}
"-" {return T_MINUS;}
"=" {return T_ASSIGN;}
"{" {//printf("{\n");
       return T_LBRACE;}
"}" {//printf("}\n");
       return T_RBRACE;}

"*" {return T_STAR;}

"/" {return T_DIVIDE;}
"%" {return T_MODULO;}
"+" {return T_PLUS;}
"<<" {return T_LEFT_SHIFT;}
">>" {return T_RIGHT_SHIFT;}
"<" {return T_LESS_THAN;}
">" {return T_GREATER_THAN;}
"<=" {return T_LESS_EQUAL;}
">=" {return T_GREATER_EQUAL;}
"==" {return T_EQUAL;}
"!=" {return T_NOT_EQUAL;}
"&" {return T_BITWISE_AND;}
"|" {return T_BITWISE_OR;}
"&&" {return T_LOGICAL_AND;}
"||" {return T_LOGICAL_OR;}

"++" {return T_INCREMENT;}
"--" {return T_DECREMENT;}
";" {return T_SEMICOLON;}

"//".*             { /* ignore single line comment */ }

"#include" {
    BEGIN(DIRECTIVE); // enter directive mode
}

<DIRECTIVE>"<"[^>]*">" {
    // match #include <filename> and ignore it
    //printf("ignore:%s\n", yytext);
    BEGIN(INITIAL); // return to normal mode
}

<DIRECTIVE>"\""[^"]*"\"" {
    // match #include "filename" and ignore it
    //printf("ignore:%s\n", yytext);
    BEGIN(INITIAL); // return to normal mode
}

"/*" {
    comment_level++; // match a comment start, increment the counter
    //printf("comment_level is :%d\n",comment_level);
    BEGIN(COMMENT);  // enter comment mode
}

<COMMENT>"/*" {
    error_flag = 1;
    //printf("comment_level is :%d\n",comment_level);
}

<COMMENT>"*/" {
    comment_level--; // match a comment end, decrement the counter
    //printf("comment_level is :%d\n",comment_level);
    if (comment_level == 0) {
        BEGIN(INITIAL); // if comment_level == 0, return to normal mode
    }
}
"*/" {
   error_flag = 1;
   //printf("error\n");
}

<COMMENT>\n {
    yylineno++; // renew line
}

<COMMENT>. {
    // ignore other characters
}

[ \t\r\n]+ {
    const char *text = yytext;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') yylineno++; // renew line
    }
}

{Identifier} { 
    //printf("Identifier: %s\n", yytext);
    yylval.str = strdup(yytext);
    return T_IDENTIFIER; 
}

{decimal} {         
    yylval.str = strdup(yytext); 
    return T_DEC_CONSTANT;
}

{HEX_PREFIX}{HEX_DIGIT}+ {
    return T_HEX_CONSTANT;
}

{untermchrlit} {
    printf("!");
    printf("<T_ERROR, \"%s\", %d>\n", yytext, yylineno);
    return T_ERROR;
}

{chrlit} {
    yylval.str = strdup(yytext);
     return T_CHAR_CONSTANT;
}


. {
    printf("?");
    printf("<T_ERROR, \"%s\", %d>\n", yytext, yylineno);
    return T_ERROR;
}


<<EOF>> {
    // cjeck if unterminated comment
    if (comment_level > 0) {
        //fprintf(stderr, "Error: Unterminated comment detected.\n");
        error_flag = 1; // set error flag
    }
    return 0; // end of file
}
%%

/* 主函数
int main() {
    printf("Starting keyword scanner...\n");
    int token;
    while ((token = yylex()) != 0) { 
        // Token 流已经通过 printf 输出
    }
    printf("Finished scanning.\n");
    return 0;
}
*/



int yywrap() {
    return 1;
}
