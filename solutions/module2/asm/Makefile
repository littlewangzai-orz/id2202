.PHONY: all clean

# 编译生成所有目标
all: main-c main-asm

# 编译 main-c（C语言程序）
main-c: factorial.o main-c.o
	gcc -g factorial.o main-c.o -o main-c

main-c.o: main-c.c
	gcc -g -c main-c.c -o main-c.o

# 编译 main-asm（汇编程序）
main-asm: factorial.o main-asm.o
	gcc -g -no-pie main-asm.o factorial.o -o main-asm

main-asm.o: main-asm.asm
	nasm -felf64 -g -F dwarf main-asm.asm -o main-asm.o

# 编译 factorial.c 为单独的目标文件
factorial.o: factorial.c
	gcc -g -c factorial.c -o factorial.o

# 清理所有生成文件
clean:
	rm -f *.o main-c main-asm *~
