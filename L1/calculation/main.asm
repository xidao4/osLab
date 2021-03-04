%include "io.inc"

section .data
msg db 'please input two numbers:',0h

section .bss
sinput: resb 44

section .text
global CMAIN
CMAIN:
    mov ebp, esp; for correct debugging
    mov eax,msg
    call sprintLF
    
    call getInput
    
    call bigNumAdd
    mov eax,0Ah
    push eax
    mov eax,esp
    call sprint
    pop eax
    call bigNumMul
    
    call quit
    ret 
;----------------------------------------------------------
;----------------------------------------------------------
;before> sinput
;after>
;ebx points to last bit of first num, esi is length
;edx points to last bit of second num,edi is length
init:
    mov ebx,sinput
    mov esi,0
.firstNum:     
    cmp byte[ebx],48
    jl .interval
    cmp byte[ebx],57
    jg .interval
    inc esi
    inc ebx
    jmp .firstNum
.interval:  
    mov edx,ebx
    inc edx
    sub ebx,1
    mov edi,0 
.secondNum:
    cmp byte[edx],48
    jl .done
    cmp byte[edx],57
    jg .done
    inc edi
    inc edx
    jmp .secondNum
.done:
    sub edx,1
    ret
;----------------------------------------------------------
;----------------------------------------------------------
initEBX:
    inc ebx   
.firstNum:     
    cmp byte[ebx],48
    jl .done
    cmp byte[ebx],57
    jg .done
    inc ebx
    jmp .firstNum
.done:
    sub ebx,1
    ret        
;----------------------------------------------------------
;----------------------------------------------------------
;question: it seams main memory is filled with zero, and so is stack(otherwises how come we can print LF by push 0Ah only).
fillZeros:
    mov eax,0x804a07c   ;0x804104c+44  we can +16*3 ,so use 
    add esi,edi ;ret is no more than m+n bits
.fillNext:
    cmp esi,0
    jz .doneFill
    mov dword[eax],0
    add eax,4
    sub esi,1
    jmp .fillNext
.doneFill:
    ret      
;----------------------------------------------------------
;----------------------------------------------------------
getTotalBits:;the result is sotred in ecx
    push esi
    mov esi,sinput
    mov ebx,0
.next:    
    cmp byte[esi],10
    je .done
    inc esi
    inc ebx
    jmp .next
.done:
    pop esi
    sub ebx,1
    ret
;----------------------------------------------------------
;----------------------------------------------------------
bigNumMul:
    push esi
    push edi
    push ebx
    push edx
    push eax
    push ecx
    
    call init
    ;--------------------------ebx pointer,esi length; edx pointer,edi length------------------------------------
    call fillZeros;but it seems these space initially is zero.
       
    mov esi,0x804a07c   ;0x804104c+44    
    mov edi,0    
.outerloop:  
    cmp byte[edx],32
    je .doneOuterloop
    mov ecx,0
    mov cl,byte[edx]
    sub ecx,48    
.innerloop:
    add ebx,1
    cmp ebx,sinput
    je .doneInnerloop ;must right follow the previous sentence
    sub ebx,1
    mov eax,0
    mov al,byte[ebx]
    sub eax,48
    ;store
    ;mul ecx
    ;wrong,see >>  https://stackoverflow.com/questions/41022248/mul-and-memory-allocation-in-registers-edxeax-with-masm
    ;imul is another solution
    push edx
    mul ecx
    pop edx
    ;add byte[esi],al
    add dword[esi],eax
    add esi,4
    sub ebx,1
    jmp .innerloop
.doneInnerloop:
    sub ebx,1
    add edi,4
    mov esi,0x804a07c   
    ;store position
    add esi,edi
    call initEBX
    sub edx,1
    jmp .outerloop
    ;-----------------123*456 tested------------------------
.doneOuterloop:
    ;handle carries
    
    call getTotalBits;ret in ebx
    mov esi,0x804a07c
    mov edi,0; carry bit
    mov ecx,10;divisor
.handleCarry:
    cmp ebx,0
    jz .doneCarry
    mov eax,dword[esi]     
    add eax,edi ;dividend
    mov edx,0   ;remainder
    div ecx     ;divisor
    mov edi,eax ;move quotient to carry bit
    mov dword[esi],edx  ;move remainder to the according memory
    add esi,4
    sub ebx,1 
    jmp .handleCarry
.doneCarry:    
    call getTotalBits;ret in ebx
    mov esi,0x804a07c
    mov eax,4
    mul ebx
    sub eax,4
    add esi,eax
    ;see whether first bit is zero. if so, (maybe) don't print.  further check in .printPre
    cmp dword[esi],0
    jz .printPre
    call printBit
.printPre:
    cmp esi,0x804a07c
    jz .specialCase
    sub esi,4
.printLoop:
    cmp esi,0x804a07c
    jl .finished
    call printBit
    sub esi,4
    jmp .printLoop
.specialCase:
    call printBit            
.finished:    
    pop ecx
    pop eax
    pop edx
    pop ebx
    pop edi
    pop esi
    ret
;----------------------------------------------------------
;----------------------------------------------------------
printBit:
    mov eax,dword[esi]
    add eax,48
    push eax
    mov eax,esp
    call sprint
    pop eax
    ret
        
;----------------------------------------------------------
;----------------------------------------------------------                
bigNumAdd:
    push esi
    push edi
    push ebx
    push edx
    push eax
    push ecx
    
    call init
    ;---------------------init ebx,esi    and    edx,edi---------------------
    ;ecx:if set 1,means has carry, otherwise means no carry
    ;eax: each bit value, to push it into statck
    mov ecx,0
.addAB:
    add ebx,1
    cmp ebx,sinput
    jle .addB
    sub ebx,1
    mov eax,sinput
    add eax,esi
    cmp edx,eax
    jle .addA
    
    mov eax,0
    add al,byte[ebx]
    sub eax,48
    add al,byte[edx]
    sub eax,48
    add eax,ecx
    jmp .pushStack
.addB: 
    sub ebx,1
    mov eax,sinput
    add eax,esi
    cmp edx,eax
    jle .popStack
    mov eax,0   
    add al,byte[edx]
    sub eax,48
    add eax,ecx
    jmp .pushStack
.addA:
    mov eax,0
    add al,byte[ebx]
    sub eax,48
    add eax,ecx    
    
.pushStack:    
    cmp eax,10
    jl .noCarry
    sub eax,10
    mov ecx,1
    jmp .pushASCII
.noCarry:
    mov ecx,0
.pushASCII:    
    add eax,48
    push eax    

    sub ebx,1
    sub edx,1
    jmp .addAB
    
.popStack:
    cmp esi,edi ;esi stores bigger value
    jg .continue
    mov esi,edi
.continue:
    cmp ecx,0
    jz .popLoop
    add ecx,48
    push ecx
    mov eax,esp
    call sprint
    pop ecx
.popLoop:
    cmp esi,0
    jz .finished
    mov eax,esp
    call sprint
    pop eax
    sub esi,1
    jmp .popLoop               
.finished:  
    pop ecx
    pop eax  
    pop edx
    pop ebx
    pop edi    
    pop esi
    ret     
            
;------------------------------------------------------------
;------------------------------------------------------------    
quit:
    mov eax,1
    mov ebx,0
    int 80h
    ret   
;----------------------------------------------------------
;----------------------------------------------------------    
sprintLF:;argument is addr,in eax
    call sprint     
    push eax
    mov eax,0Ah
    push eax
    mov eax,esp
    call sprint
    pop eax
    pop eax
    ret
sprint:;argument is addr,in eax
    push edx
    push ecx
    push ebx
    push eax
    
    mov ecx,eax ;ecx: addr
    
    call getStrlen
    mov edx,eax ;edx: length

    mov ebx,1   ;ebx: STDOUT
    
    mov eax,4
    int 80h
    
    pop eax
    pop ebx
    pop ecx
    pop edx
    ret
;----------------------------------------------------------------
;----------------------------------------------------------------       
getStrlen:;argument is addr,in eax
    push ebx
    mov ebx,eax
nextchar:    
    cmp byte[eax],0
    jz finished
    inc eax
    jmp nextchar
finished:
    sub eax,ebx
    pop ebx
    ret 
;---------------------------------------------------------------
;---------------------------------------------------------------    
getInput:;input result in ecx
    push eax
    push ebx
    push edx
    
    mov ecx,sinput ;addr
    mov eax,3
    mov ebx,0   ;STDIN
    mov edx,44  ;len
    int 80h       
    
    pop edx
    pop ebx
    pop eax
    ret   