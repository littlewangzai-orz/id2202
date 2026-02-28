# set the compiler
CC = gcc

# FLex ºÍ Bison file
FLEX = scanner.l
BISON = parser.y

# generate the source code file
LEX_C = lex.yy.c
BISON_C = parser.tab.c
BISON_H = parser.tab.h

# source code file
AST_C = ./src/ast.c
AST_H = ./include/ast.h

ENV_C = ./src/env.c
ENV_H = ./include/env.h

ACC_C = ./src/acc.c
ACC_H = ./include/acc.h

ASM_IR_C = ./src/ASM_IR.c
ASM_IR_H = ./include/ASM_IR.h

HYBRID_C = ./src/hybrid_IR.c
HYBRID_H = ./include/hybrid_IR.h

reg2stack_C = ./src/reg2stack.c
reg2stack_H = ./include/reg2stack.h
MAIN_C = main.c

# object file
OBJS = $(BISON_C) $(LEX_C) $(AST_C) $(ENV_C) $(ACC_C) $(ASM_IR_C) $(reg2stack_C) $(HYBRID_C) $(MAIN_C)

# executable file
TARGET = cigrid

# default target
all: $(TARGET)

# generate the executable file	
$(TARGET): $(OBJS)
	$(CC) -w -o $@ $(OBJS) -lfl

# compile the bison file
$(BISON_C) $(BISON_H): $(BISON)
	bison -d $<

# compile the flex file
$(LEX_C): $(FLEX)
	flex $<

# clean the object file
clean:
	rm -f $(TARGET) $(BISON_C) $(BISON_H) $(LEX_C) *.o

# rebuild the executable file
rebuild: clean all
