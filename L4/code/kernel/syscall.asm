
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"
extern sys_P
extern sys_V
extern disp_color_str
extern sys_print_color
extern sys_process_sleep

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
INT_VECTOR_SYS_CALL equ 0x90
_NR_sys_sleep equ 1
_NR_sys_disp_color_str equ 2
_NR_sys_p equ 3
_NR_sys_v equ 4

; 导出符号
global	get_ticks
global  sys_sleep
global sys_sleep_helper
global  sys_disp_color_str
global  sys_p
global  sys_v
global sys_Phelper
global sys_Vhelper
global  sys_disp_str_helper


bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

sys_sleep:
	push ebx
	mov ebx,[esp+8]
	mov eax,_NR_sys_sleep
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
	
sys_sleep_helper:
	push ebx
	call sys_process_sleep
	pop ebx
	ret

sys_disp_str_helper:
	push ebx
	push ecx
	call sys_print_color
	pop ecx
	pop ebx
	ret
sys_disp_color_str:
	push ebx
	push ecx
	mov ebx,[esp+16]
	mov ecx,[esp+12]
	mov eax,_NR_sys_disp_color_str
	int INT_VECTOR_SYS_CALL
	pop ecx
	pop ebx
	ret

sys_p:
	push ebx
	mov ebx,[esp+8]
	mov eax,_NR_sys_p
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
sys_Phelper:
	push ebx
	call sys_P
	pop ebx
	ret
sys_Vhelper:
	push ebx
	call sys_V
	pop ebx
	ret
sys_v:
	push ebx
	mov ebx,[esp+8]
	mov eax,_NR_sys_v
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret