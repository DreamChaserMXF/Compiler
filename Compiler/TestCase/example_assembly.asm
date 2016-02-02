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
    _string_format       db '%s ', 0
    _String0           db 'the max add 1 is', 0
    _String1           db 'the max is', 0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   16

    ; 25       READ                0               0             c#0
    lea     eax, SS:[ebp - 4]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 26       READ                0               0             d#1
    lea     eax, SS:[ebp - 8]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 27      WRITE                0               0             c#0
    mov     eax, SS:[ebp - 4]
    push    eax
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ; 28        ADD              c#0               1         _temp#0
    mov     eax, SS:[ebp - 4]
    add     eax, 1
    mov     SS:[ebp - 16], eax
    ; 29      WRITE                0               0         _temp#0
    mov     eax, SS:[ebp - 16]
    push    eax
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 30       SETP                0               0             c#0
    push    SS:[ebp - 4]
    ; 31       SETP                0               0             d#1
    push    SS:[ebp - 8]
    ; 32  FUNC_CALL                0               0               3
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _calcuMax3
    add     esp, 12
    ; 33      STORE                0               0           max#2
    mov     SS:[ebp - 12], eax
    ; 34      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 35      WRITE                0               0           max#2
    mov     eax, SS:[ebp - 12]
    push    eax
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 36      WRITE                0               0             c#0
    mov     eax, SS:[ebp - 4]
    push    eax
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ; 37      WRITE                0               0             d#1
    mov     eax, SS:[ebp - 8]
    push    eax
    push    offset  _char_format_p
    call    printf
    add     esp, 8

    add     esp,   16
    pop     ebp
    pop     edx
    pop     ebx
    pop     eax
    call    ExitProcess
_main  endp


    ;  1      BEGIN                0               0               3
_calcuMax3  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   4

    ; 12        JNG              x#4             y#5               2
    mov     eax, SS:[ebp + 16]
    mov     edx, SS:[ebp + 12]
    cmp     eax, edx
    jng     _label2
    ; 13        ASG              x#4               0           max#6
    mov     eax, SS:[ebp + 16]
    mov     SS:[ebp - 12], eax
    ; 14        JMP                0               0               3
    jmp     _label3
    ; 15      LABEL                0               0               2
_label2:
    ; 16        ASG              y#5               0           max#6
    mov     eax, SS:[ebp + 12]
    mov     SS:[ebp - 12], eax
    ; 17      LABEL                0               0               3
_label3:
    ; 18        RET                0               0           max#6
    mov     eax, SS:[ebp - 12]
    jmp     calcuMax3_Exit
    ; 19        ADD              x#4           max#6             x#4
    mov     eax, SS:[ebp + 16]
    mov     edx, SS:[ebp - 12]
    add     eax, edx
    mov     SS:[ebp + 16], eax
    ; 20        ADD              y#5               1             y#5
    mov     eax, SS:[ebp + 12]
    add     eax, 1
    mov     SS:[ebp + 12], eax
    ; 21       SETP                0               0             c#0
    push    SS:[ebp + 8]
    push    SS:[ebx - 4]
    ; 22       SETP                0               0             d#1
    push    SS:[ebp + 8]
    push    SS:[ebx - 8]
    ; 23  PROC_CALL                0               0               7
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 4], eax
    mov     SS:[esp - 8], ebp
    sub     esp, 8
    call    _calcuMax17
    add     esp, 16
    ; 24        END                0               0               3
calcuMax3_Exit:
    add     esp,   4
    pop     ebp
    ret
_calcuMax3  endp

    ;  2      BEGIN                0               0               7
_calcuMax17  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   4

    ;  3        JNG              x#8             y#9               0
    mov     eax, SS:[ebp + 20]
    mov     edx, SS:[ebp + 16]
    cmp     eax, edx
    jng     _label0
    ;  4      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  5      WRITE                0               0             x#8
    mov     eax, SS:[ebp + 20]
    push    eax
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ;  6        JMP                0               0               1
    jmp     _label1
    ;  7      LABEL                0               0               0
_label0:
    ;  8      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  9      WRITE                0               0             y#9
    mov     eax, SS:[ebp + 16]
    push    eax
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ; 10      LABEL                0               0               1
_label1:
    ; 11        END                0               0               7
calcuMax17_Exit:
    add     esp,   4
    pop     ebp
    ret
_calcuMax17  endp

end start
