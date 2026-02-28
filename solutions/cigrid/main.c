#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./include/ast.h"
#include "./include/hybrid_IR.h"
#include "./include/acc.h"
#include "./include/env.h"

// Declare external parser function and program variable
extern int yyparse();
extern int error_flag;
extern FILE *yyin; // Input file for the parser
extern ASTNode* program; // Parsed program output

int main(int argc, char *argv[]) {
    int pretty_print_flag = 0; // Flag to determine if pretty-print is required
    int asm_flag = 0;
    char *input_file = NULL;  // Path to the input file
    //initialize the environment 
    Environment env;
    init_env(&env);

    //initialize the accumlator 
    Accumulator acc;
    init_accumulator(&acc);

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pretty-print") == 0) {
            pretty_print_flag = 1; // Set the pretty-print flag
        } 
        else if(strcmp(argv[i],"--asm") == 0){
            asm_flag = 1;
        }
        else if (input_file == NULL) {
                input_file = argv[i]; // Treat any parameter as a file if it's not a flag
        } else {
            
            return 1;
        }

    }
    // Handle empty file or no input file gracefully
    if (input_file == NULL) {
        printf("No input file provided. Using standard input (stdin).\n");
        return 0; // Use standard input as the input file
    } else {
        // Open the specified input file
        yyin = fopen(input_file, "r");
        
        if (yyin == NULL) {
            fprintf(stderr, "Error: Unable to open file %s.\n", input_file);
            return 1;
        }

        // Check if the file is empty
        fseek(yyin, 0, SEEK_END);
        long file_size = ftell(yyin);
        rewind(yyin);

        if (file_size == 0) {
            
            if (pretty_print_flag) {
                printf("Pretty-print mode enabled, but nothing to print.\n");
            }
            fclose(yyin);
            return 0; // Return success code even for an empty file
        }
       
    }

    // Perform syntax analysis

    if (yyparse() == 0) {
        
        if (pretty_print_flag) {
            //If pretty-print is enabled, output formatted program
               if(!error_flag){
                    if(program!=NULL)
                        {
                           
                            int n = 0;
                            IR_Global* ir = convertASTToIR(program->children[0]);
                            printIRGlobal(ir);
                            
                            hybrid2ASM(&env, &n, &acc, ir);

                            print_accumulator(&acc);

                            free(program);
                            free_accumulator(&acc);
                            free_env(&env);
                            return 0; // return success
                        }
               }
               else {
                    printf("error!\n");
                    return 1;
                }
        } 
        else if (asm_flag){
            int n = 0;
                            IR_Global* ir = convertASTToIR(program->children[0]);

                            hybrid2ASM(&env, &n, &acc, ir);

                            register_spilling(acc,n);
                            
                            free(program);
                            free_accumulator(&acc);
                            free_env(&env);
                            return 0; // return success
        }
        else {

            return 1;
        }
    } else {

        if (input_file != NULL) {
            fclose(yyin);
        }
        return 1;
    }

    // Close the input file if it is not stdin
    if (input_file != NULL) {
        fclose(yyin);
    }

    return 0;
}
