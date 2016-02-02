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
    _String0           db 'begin', 0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   4

    ;  7  FUNC_CALL                0               0               0
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _func0
    add     esp, 4
    ;  8      STORE                0               0         _temp#0
    mov     SS:[ebp - 4], eax
    ;  9      WRITE                0               0         _temp#0
    mov     eax, SS:[ebp - 4]
    push    eax
    push    offset  _integer_format
    call    printf
    add     esp, 8

    add     esp,   4
    pop     ebp
    pop     edx
    pop     ebx
    pop     eax
    call    ExitProcess
_main  endp


_func0  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   8

    ;  2       AASG               99               1           seq#1
    mov     eax, 99
    mov     SS:[ebp - 8], eax
    ;  3      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  4        RET                0               1           seq#1
    mov     eax, SS:[ebp - 8]
    jmp     func0_Exit
    ;  5      WRITE                0               0               1
    push    1
    push    offset  _integer_format
    call    printf
    add     esp, 8
func0_Exit:
    add     esp,   8
    pop     ebp
    ret
_func0  endp

end start
