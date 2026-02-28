global main
extern puts
extern putchar
extern atoi
extern factorial_message

section .data
menu_msg: db "usage: <command> <arg1> <arg2>", 10
          db "commands:", 10
          db "  e   Echo arguments arg1 and arg2", 10
          db "  m   Prints out a magic number", 10
          db "  +   Adds together arg1 and arg2", 10
          db "  !   Prints out the factorial value of arg1, as well as the message in arg2", 0 

magic_number: dq -126

section .text



main:

    mov rax, rdi        ; argc number in rax
    cmp rax, 4          ; cmp rax 4
    jl call_menu        ; if argc < 4,jump to call_menu

   
    mov rbx, rsi        ; argv in rsi 
    mov rdi, [rbx + 8]  ; take argv[1]（command 的地址） rdi has been destoryed , the argc number is in rax
    mov al, byte [rdi]  ; take command[0]



    cmp al, 'e'         ;judge whether is 'e'
    je handle_echo      ;if yes, then jump to handle_echo


    cmp al, 'm'         ;judge whether is 'm'
    je handle_magic     ;if yes, then jump to handle_magic

    cmp al, '+'
    je handle_add

    cmp al, '!'
    je handle_factorial

    ; argc != + e m or anything else return false
    call_menu

call_menu:
    call menu           ; 调用 menu 函数
    mov rax, 1          ; 设置返回码 1
    ret

    
handle_echo:
    push rbp
    mov rbp, rsp

    ; 打印 argv[2]
    mov rdi, [rbx + 16]  ; 取 argv[2] 的地址
    call puts            ; 打印 argv[2]

    ; 打印 argv[3]
    mov rdi, [rbx + 24]  ; 取 argv[3] 的地址
    call puts            ; 打印 argv[3]

    ; 明确返回值
    xor rax, rax         ; 设置返回值为 0
    pop rbp
    ret

    

handle_add:
    ; 保存原始栈帧
    push rbp
    mov rbp, rsp

    ; 转换 argv[2] 为整数
    push rax
    mov rdi, [rbx + 16] ; 加载 argv[2] 的地址到 rdi
    call atoi           ; 调用 atoi 转换为整数
    mov r10, rax         ; 将结果存储到 r8
    pop rax

    ; 转换 argv[3] 为整数
    push r10
    mov rdi, [rbx + 24] ; 加载 argv[3] 的地址到 rdi
    call atoi           ; 调用 atoi 转换为整数
    pop r10

    ; 将两个整数相加
    add rax, r10         ; rax = r8 + rax（r8 是 argv[2]，rax 是 argv[3]）

    ; 打印结果
    mov rdi, rax        ; 将结果作为参数传递给 print_int
    call print_int      ; 调用 print_int 打印结果

    pop rbp
    ret

handle_magic:
    mov rdi, qword [magic_number] 
    call print_int                
    ret

handle_factorial:

    mov rdi, [rbx + 16]  ; 取 argv[2] 的地址
    call atoi            ; 转换为整数，结果在 rax
    mov rdi, rax         ; 将数字存入 rdi

    
    mov rsi, [rbx + 24]  ; 取 argv[3] 的地址

    ; 调用 factorial_message
    call factorial_message

    ; 打印返回的阶乘结果
    mov rdi, rax         ; 函数返回值在 rax
    call print_int

    ret

menu:
    push rbp
    mov rbp, rsp

    mov rdi, menu_msg   ; 将完整字符串的地址加载到 rdi
    call puts           ; 调用 puts 打印字符串

    ; 设置返回码 1 表示错误
    mov rax, 1          ; 错误返回码放入 rax

    pop rbp
    ret                 ; 返回调用者


print_int:
    push rbp
    mov rbp, rsp

    ; 保存参数 x 到 rax
    mov rax, rdi     ; rdi 是传入的整数参数

    ; 检查是否是负数
    cmp rax, 0
    jge .handle_positive ; 如果 x >= 0，跳转处理正数

    ; 打印负号
    push rax             ; 保存 rax
    mov rdi, '-'         ; 加载负号字符到 rdi
    call putchar         ; 调用 putchar 打印负号
    pop rax              ; 恢复 rax 原始值


    ; 取绝对值
    neg rax

.handle_positive:
    ; 初始化变量 i = 1000000
    mov rcx, 1000000      ; rcx 用作当前的除数

    ; 初始化 b = 0
    xor r8, r8            ; r8 是 b，初始值为 0

.print_loop:
    ; check i == 0?
    cmp rcx, 0
    je .done              ; 如果 i == 0，退出循环
    
    ; 计算 x / i
    xor rdx, rdx          ; 清除 rdx
    div rcx               ; rax / rcx -> 商在 rax，余数在 rdx

    ; 判断是否需要打印
    cmp rax, 0
    jne .print_digit      ; 如果商非 0，跳转打印数字

    ; 如果 b == 1 或 i == 1，打印 '0'
    cmp r8, 0
    jne .print_digit
    cmp rcx, 1
    jne .skip_digit

.print_digit:
    ; 打印数字字符 '0' + 商
    add rax, '0'
    
    push rdx
    push rax  
    push rcx           ; 保存 rax
    mov rdi, rax        ; 加载负号字符到 rdi
    call putchar         ; 调用 putchar 打印负号
    pop rcx
    pop rax              ; 恢复 rax 原始值
    pop rdx

    ; 设置 b = 1
    mov r8, 1

.skip_digit:

    mov rax,rdx
    xor rdx, rdx        ; 清除高位 rdx，准备执行除法
    div rcx             ; 执行无符号除法：rax / rcx
                    ; 结果：商在 rax，余数在 rdx
    mov rax, rdx        ; 更新 rax 为余数，即 x = x % i

    ; 计算 i / 10
    push rax            ; 保存 rax，因为 rax 当前是 x = x % i
    mov rax, rcx        ; 将当前的 i（rcx）存储到 rax
    xor rdx, rdx        ; 清除高位 rdx
    mov rbx, 10         ; 加载常数 10
    div rbx             ; 执行除法：rax / 10
    mov rcx, rax        ; 更新 rcx 为新的 i，即 i = i / 10
    pop rax             ; 恢复 rax，为 x = x % i 的值

    jmp .print_loop

.done:
    ; 打印换行符
    push rax
    mov rdi, 10
    call putchar
    pop rax
    ; 恢复栈帧
    pop rbp
    ret