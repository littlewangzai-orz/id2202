#include "parser.hpp"
#include "ir.hpp"
#include "asm_ir.hpp"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    exit(1);
  }

  bool print_tree = false;
  bool report_err = false;
  bool print_ir = false;
  bool asm_ir = false;
  bool write_asm = false;
  std::string print_flag {"--pretty-print"};
  std::string err_flag {"--line-error"};
  std::string ir_flag {"--print-ir"};
  std::string asm_ir_flag {"--asm-ir"};
  std::string asm_flag {"--asm"};

  for (int i= 2; i < argc;i++){
    if (print_flag == argv[i-1]){
      print_tree = true;
    }else if (err_flag == argv[i-1]){
      report_err = true;
    }else if (ir_flag == argv[i-1]){
      print_ir = true;
    }else if (asm_ir_flag == argv[i-1]){
      asm_ir = true;
    }else if (asm_flag == argv[i-1]){
      write_asm = true;
    }else{
      std::cout << "Unknown flag: " << argv[i-1] << std::endl; 
      exit(1);
    }
  }

  std::ifstream codefile(argv[argc-1]);
  if (!codefile.is_open()) {
    exit(1);
  }

  auto tokens = tokenize(codefile,report_err);

  Parser p{std::move(tokens),report_err};
  auto global_nodes = p.parse_tokens();

  if (print_tree){
    print_ast(p,global_nodes);
  }
  
  if (!print_ir && !asm_ir && !write_asm){
    return 0;
  }

  IRRepr repr(std::move(p),std::move(global_nodes));

  if (print_ir){
    repr.print_IR();
  }

  if (!asm_ir && !write_asm){
    return 0;
  }

  auto asm_globals = instruction_selection(repr);

  if (asm_ir){
    print_asm(asm_globals);
  }

  if (write_asm){
    spill(asm_globals);
    print_asm(asm_globals);
  }

}
