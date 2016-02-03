TITLE MXF_AssemblyCode

.386
.model flat, stdcall
option casemap: none

includelib .\masm32\lib\msvcrt.lib
includelib .\masm32\lib\kernel32.lib
include .\masm32\include\msvcrt.inc
include .\masm32\include\kernel32.inc
printf PROTO C: ptr sbyte, :vararg
scanf  PROTO C: ptr sbyte, :vararg

.STACK

.DATA
    _integer_format_s    db '%d' , 0   ; for scanf
    _integer_format_p    db '%d ', 0   ; for printf
    _char_format_s       db '%c' , 0   ; for scanf
    _char_format_p       db '%c ', 0   ; for printf
    _string_format       db '%s', 0
    _String0             db 70,105,98,40,110,41,32,105,115,58,32,0
    _String1             db 80,108,101,97,115,101,32,101,110,116,101,114,32,97,32,110,117,109,98,101,114,32,110,32,119,104,105,99,104,32,105,115,32,108,97,114,103,101,114,32,116,104,97,110,32,48,58,32
	                     db 0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   8

    ; 25        ASG               -1               0             n#0
    mov     eax, -1
    mov     SS:[ebp - 4], eax
    ; 26      LABEL                0               0               4
_label4:
    ; 27        JNL              n#0               0               5
    mov     eax, SS:[ebp - 4]
    mov     edx, 0
    cmp     eax, edx
    jnl     _label5
    ; 28      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 29       READ                0               0             n#0
    lea     eax, SS:[ebp - 4]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ; 30        JMP                0               0               4
    jmp     _label4
    ; 31      LABEL                0               0               5
_label5:
    ; 32      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 33       SETP                0               0             n#0
    push    dword ptr SS:[ebp - 4]
    ; 34  FUNC_CALL                0               0               2
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _fib2
    add     esp, 8
    ; 35      STORE                0               0        result#1
    mov     SS:[ebp - 8], eax
    ; 36      WRITE                0               0        result#1
    push    dword ptr SS:[ebp - 8]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8

    add     esp,   8
    pop     ebp
    pop     edx
    pop     ebx
    pop     eax
    call    ExitProcess
_main  endp


    ;  1      BEGIN                0               2               2
_fib2  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   8

    ;  2        JNE              k#3               0               0
    mov     eax, SS:[ebp + 12]
    mov     edx, 0
    cmp     eax, edx
    jne     _label0
    ;  3        RET                0               0               1
    mov     eax, 1
    jmp     fib2_Exit
    ;  4        JMP                0               0               1
    jmp     _label1
    ;  5      LABEL                0               0               0
_label0:
    ;  6        JNE              k#3               1               2
    mov     eax, SS:[ebp + 12]
    mov     edx, 1
    cmp     eax, edx
    jne     _label2
    ;  7      WRITE                0               0               1
    push    1
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ;  8        RET                0               0               1
    mov     eax, 1
    jmp     fib2_Exit
    ;  9        JMP                0               0               3
    jmp     _label3
    ; 10      LABEL                0               0               2
_label2:
    ; 11        SUB              k#3               1         _temp#0
    mov     eax, SS:[ebp + 12]
    sub     eax, 1
    mov     SS:[ebp - 4], eax
    ; 12       SETP                0               0         _temp#0
    push    dword ptr SS:[ebp - 4]
    ; 13  FUNC_CALL                0               0               2
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 4], eax
    sub     esp, 4
    call    _fib2
    add     esp, 8
    ; 14      STORE                0               0         _temp#0
    mov     SS:[ebp - 4], eax
    ; 15        SUB              k#3               2         _temp#1
    mov     eax, SS:[ebp + 12]
    sub     eax, 2
    mov     SS:[ebp - 8], eax
    ; 16       SETP                0               0         _temp#1
    push    dword ptr SS:[ebp - 8]
    ; 17  FUNC_CALL                0               0               2
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 4], eax
    sub     esp, 4
    call    _fib2
    add     esp, 8
    ; 18      STORE                0               0         _temp#1
    mov     SS:[ebp - 8], eax
    ; 19        ADD          _temp#0         _temp#1             n#0
    mov     eax, SS:[ebp - 4]
    add     eax, SS:[ebp - 8]
    mov     ebx, SS:[ebp + 8]
    mov     SS:[ebx - 4], eax
    ; 20      WRITE                0               0             n#0
    push    dword ptr SS:[ebp + 8]
    push    dword ptr SS:[ebx - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 21        RET                0               0             n#0
    mov     eax, SS:[ebp + 8]
    mov     eax, SS:[ebx - 4]
    jmp     fib2_Exit
    ; 22      LABEL                0               0               3
_label3:
    ; 23      LABEL                0               0               1
_label1:
    ; 24        END                0               0               2
fib2_Exit:
    mov     esp, ebp
    pop     ebp
    ret
_fib2  endp

end start
