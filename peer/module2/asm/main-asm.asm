
        global  main
        extern  puts
        extern  putchar
        extern  atoi
        extern  factorial_message
        extern  fflush

        section .text

menu_str    db "usage: <command> <arg1> <arg2>",10,10 \
    ,"commands:",10 \
    ,"  e   Echo arguments arg1 and arg2",10 \
    ,"  m   Prints out a magic number",10 \
    ,"  +   Adds together arg1 and arg2",10 \
    ,"  !   Prints out the factorial value of",10 \
    ,"      arg1, as well as the message in arg2",10,0
e           db "e",0
m           db "m",0
plus        db "+",0
excl        db "!",0

main:
        mov    rbp, rdi ; get argc
        mov    rbx, rsi ; get argv
        cmp    rbp, 4
        jl     exit_menu
        mov    rdi, [rbx+8] ; second argv
        mov    rsi, e
        mov    ecx, 1
        repe   cmpsb
        jne    eq_m
        mov    rsi, rbx
        jmp    echo
eq_m:
        mov    rdi, [rbx+8] ; second argv
        mov    rsi, m
        mov    ecx, 1
        repe   cmpsb
        jne    eq_plus
        mov    rdi, -126
        call   print_int
        mov    eax, 0
        ret
eq_plus:
        mov    rdi, [rbx+8] ; second argv
        mov    rsi, plus
        mov    ecx, 1
        repe   cmpsb
        jne    eq_excl
        push   rbx           ; save argv ptr
        mov    rdi, [rbx+16] ; third argv
        call   atoi
        pop    rbx    ; get argv ptr
        push   rax  ; push result of first atoi
        mov    rdi, [rbx+24] ; fourth argv
        call   atoi
        pop    rdi  ; get result of first atoi
        add    rdi, rax ; add together
        call   print_int
        mov    eax, 0
        ret
eq_excl:
        mov    rdi, [rbx+8] ; second argv
        mov    rsi, excl
        mov    ecx, 1
        repe   cmpsb
        jne    exit
        push   rbx           ; save argv ptr
        mov    rdi, [rbx+16] ; third argv
        call   atoi
        pop    rbx    ; get argv ptr
        mov    rsi, [rbx+24] ; fourth argv
        mov    rdi, rax ; atoi result as first arg
        call   factorial_message
        mov    rdi,rax ; move result to rdi
        call   print_int
        mov    eax, 0
        ret
exit:
        jmp    exit_menu
        ret

print_int:
        mov     rbx, rdi; grab int to print ; x = rbx
        cmp     rbx, 0
        jge     print_nums
        mov     rdi, '-'
        push    rbx   
        call    putchar
        pop     rax
        mov     rdx, 0  ; x = x* -1
        mov     r12, -1
        imul    r12
        mov     rbx, rax
print_nums:
        mov     r10, 1000000  ; i = r10
        mov     r11, 0    ; b = r11
int_loop:
        cmp     r10, 0   ; i != 0
        je      print_int_exit
        cmp     rbx, r10   ; x >= i
        jge     inner_if
        cmp     rbx, 0   ; x == 0
        sete    r12b
        cmp     r10, 1  ; i == 1 
        sete    r13b
        and     r12b,r13b  ; &&
        cmp     r12b,1
        je      inner_if
        cmp     r11,0   ; b > 0
        jg      inner_if
        jmp     after_if
inner_if:
        mov     rax, rbx 
        mov     rdx, 0
        idiv    r10     ; x / i
        mov     rdi, '0'
        add     rdi, rax ; add div result to '0'
        push    rbx    ; save rbx and r10
        push    r10
        call    putchar
        pop     r10
        pop     rbx
        mov     r11, 1     
after_if:
        mov     rax, rbx
        mov     rdx, 0
        idiv    r10     ; x / i
        mov     rbx, rdx ; x = take remainder
        mov     rax, r10  
        mov     rdx,0   ; remember rdx:rax is top of div
        mov     r12,10  ; i / 10
        idiv    r12
        mov     r10, rax ; set i 
        jmp     int_loop
print_int_exit:
        mov     rdi, 10 ; print \n
        call    putchar
        ret

echo:
        mov     rbx, rsi
        mov     rdi, [rbx+16]
        call    puts
        mov     rdi, 0
        call    fflush
        mov     rdi, [rbx+24]
        call    puts
        mov     rdi, 0
        call    fflush
        mov     eax, 0
        ret
        
exit_menu:
        mov     rdi, menu_str
        call    puts
        mov     rdi, 0
        call    fflush
        mov     eax, 1 ; exit code 1
        ret 
        
