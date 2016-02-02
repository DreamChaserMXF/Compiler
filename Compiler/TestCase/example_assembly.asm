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
    _integer_format    db '%d ', 0
    _char_format       db '%c ', 0
    _string_format     db '%s ', 0
    _String0           db 'I like writing', 0
    _String1           db 'print calling', 0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   48

    ;  6       AASG                5               5           seq#2
    mov     eax, 5
    mov     SS:[ebp - 24], eax
    ;  7      WRITE                0               5           seq#2
    mov     eax, SS:[ebp - 24]
    push    eax
    push    offset  _integer_format
    call    printf
    add     esp, 8
    ;  8      WRITE                0               0               1
    push    1
    push    offset  _integer_format
    call    printf
    add     esp, 8
    ;  9      WRITE                0               0             318
    push    318
    push    offset  _integer_format
    call    printf
    add     esp, 8
    ; 10        ASG              109               0      lastChar#3
    mov     eax, 109
    mov     SS:[ebp - 44], eax
    ; 11  PROC_CALL                0               0               4
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _Print
    add     esp, 4
    ; 12      WRITE                0               0      lastChar#3
    mov     eax, SS:[ebp - 44]
    push    eax
    push    offset  _char_format
    call    printf
    add     esp, 8
    ; 13      WRITE                0               0              99
    push    99
    push    offset  _char_format
    call    printf
    add     esp, 8
    ; 14        ASG            seq#2               5         _temp#0
    mov     eax, SS:[ebp - 24]
    mov     SS:[ebp - 48], eax
    ; 15        ADD              109         _temp#0         _temp#0
    mov     eax, SS:[ebp - 48]
    add     eax, 109
    mov     SS:[ebp - 48], eax
    ; 16      WRITE                0               0         _temp#0
    mov     eax, SS:[ebp - 48]
    push    eax
    push    offset  _integer_format
    call    printf
    add     esp, 8
    ; 17      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8

    add     esp,   48
    pop     ebp
    pop     edx
    pop     ebx
    pop     eax
    call    ExitProcess
_main  endp


_Print  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   0

    ;  2      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  3      WRITE                0               0      lastChar#3
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 44]
    push    eax
    push    offset  _char_format
    call    printf
    add     esp, 8
    ;  4      WRITE                0               5           seq#2
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 24]
    push    eax
    push    offset  _integer_format
    call    printf
    add     esp, 8

    add     esp,   0
    pop     ebp
    ret
_Print  endp

end start
