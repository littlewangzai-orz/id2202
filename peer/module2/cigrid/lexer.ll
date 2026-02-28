%option lexer=scanner
%option outfile=scanner.cpp
%option header-file=scanner.hpp
%option token-type=Token
%option unicode
%option c++

%top{
#ifndef TOKEN_DEFINED
#define TOKEN_DEFINED
#include <iostream>
#include <fstream>
#include <sstream>
#include <variant>
#include <string>

class Token;

enum class token_type {
    IDENT,
    CHAR,
    UINT,
    STRING,
    OPERATOR,
    KEYWORD,
    END_OF_FILE,
};

typedef std::variant<std::string,int> token_value;

class Token {
    public:
        token_type token_t;
        token_value value;
        int line;
        int startc;
        int endc;

        Token() = delete;

        Token(token_type t, token_value&& v,int l, int sc){
            token_t = t;
            value = v;
            line = 0;
            startc = 0;
            endc =0;
        }

        Token(token_type t, token_value&& v,int l,int sc, int ec){
            token_t = t;
            value = v;
            line = l;
            startc = sc;
            endc = ec;
        }
};

#endif
%}

KEYWORD1    \<int\>|\<void\>|\<extern\>|\<break\>|\<new\>|\<while\>
KEYWORD2    \<char\>|\<for\>|\<return\>|\<delete\>|\<if\>|\<struct\>|\<else\>
KEYWORD     {KEYWORD1}|{KEYWORD2}
OPERATOR1  [\.,:;\(\)\{\}\[\]=\*!~\-\+\/%<>&\|]
OPERATOR2  "<="|">="|"=="|"!="|"&&"|"||"|"<<"|">>"|"++"|"--"
IDENTIFIER \b[_a-zA-Z][_a-zA-Z0-9]*\b
INT        0|[1-9][0-9]*
HEX        0[xX][0-9a-fA-F]+
CHARACTER  [\N--[\t\\\'\"]]|" "|\\n|\\t|\\\\|\\\'|\\\"
CHAR       \'{CHARACTER}\'
STRING     \"{CHARACTER}*\"
%%

\/\/\N*\n {continue;}
#\N*\n {continue;}
(?s:"/*"(.|\n)*?"*/") {continue;}
//(?s:\/\*[^\*/\/]*\*\/) {continue;}
\n      {continue;}

[ \t\r] {continue;}
{OPERATOR2} {Token t(token_type::OPERATOR,str(),lineno(),columno(),columno_end()); return t;}
{OPERATOR1} {Token t(token_type::OPERATOR,str(),lineno(),columno(),columno_end()); return t;}
{KEYWORD} {Token t(token_type::KEYWORD,str(),lineno(),columno(),columno_end()); return t;}
{INT}       {Token t(token_type::UINT,stoi(str()),lineno(),columno(),columno_end()); return t;}
{HEX}       {std::istringstream hex{str()}; int i = 0; hex >> std::hex >> i; Token t(token_type::UINT,i,lineno(),columno(),columno_end()); return t;}
{CHAR}      {std::string txt = str();Token t(token_type::CHAR,txt.substr(1,txt.size()-2),lineno(),columno(),columno_end()); return t;}
{STRING}    {std::string txt = str();Token t(token_type::STRING,txt.substr(1,txt.size()-2),lineno(),columno(),columno_end()); return t;}
{IDENTIFIER} {Token t(token_type::IDENT,str(),lineno(),columno(),columno_end()); return t;}
<<EOF>>     {Token t(token_type::END_OF_FILE,0,lineno(),columno(),columno_end()); return t;}
.           {std::ostringstream msg; msg << "Lexing Error on "<< lineno() << ":"<< columno(); throw std::runtime_error(msg.str());}

%%

#ifdef SCANNER_MAIN
int main(int argc, char* argv[]){

    if (argc != 2){
        exit(2);
    }

    std::ifstream codefile (argv[1]);
    if (!codefile.is_open()){
        exit(2);
    }

    scanner s(codefile);
    while (true){
        try {
                Token token = s.lex();
                switch (token.token_t){
                    case token_type::END_OF_FILE:
                        exit(0);
                        break;
                    case token_type::IDENT:
                        std::cout << "IDENT(" << std::get<std::string>(token.value) << ") ";
                        break;
                    case token_type::CHAR:
                        std::cout << "\'" << std::get<std::string>(token.value) << "\' ";
                        break;
                    case token_type::STRING:
                        std::cout << "\"" << std::get<std::string>(token.value) << "\" ";
                        break;
                    case token_type::UINT:
                        std::cout << "int(" << std::get<int>(token.value) << ") ";
                        break;
                    case token_type::KEYWORD:
                    case token_type::OPERATOR:
                        std::cout << std::get<std::string>(token.value) << " ";
                        break;
                }
        }catch (std::runtime_error err) {
            std::cout << err.what() << std::endl;
            exit(1);
        }
    }
}
#endif
