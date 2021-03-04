global myPrint

section .data
red:      db  1Bh, '[31;1m', 0
.len            equ $ - red
white:  db  1Bh, '[37;0m', 0
.len            equ $ - white

section .text
;(char *addr,int len,int red)
myPrint:
    mov edi,[esp+12]    ;esp+12 the 3rd parameter:red
    cmp edi,0
    je pwhite
pred:
    mov eax,4
    mov ebx,1
    mov ecx,red
    mov edx,red.len
    int 80h
    jmp end
pwhite:
    mov eax,4           ;opcode sys_write
    mov ebx,1           ;STDOUT
    mov ecx,white       ;addr
    mov edx,white.len   ;len
    int 80h
end:
    mov eax,4
    mov ebx,1
    mov ecx,[esp+4]     ;esp+4 the 1st parameter:addr
    mov edx,[esp+8]     ;esp+8 the 2rd parameter:len
    int 80h
    
    ret
