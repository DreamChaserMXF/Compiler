TITLE MXF_AssemblyCode

.386
.model flat, stdcall
option casemap: none

includelib .\masm32\lib\msvcrt.lib      ; for printf & scanf linking
includelib .\masm32\lib\kernel32.lib    ; for ExitProcess linking
include .\masm32\include\msvcrt.inc     ; 似乎没啥用，应该是printf和scanf的声明文件
include .\masm32\include\kernel32.inc   ; for ExitProcess
printf PROTO C: ptr sbyte, :vararg
scanf  PROTO C: ptr sbyte, :vararg

.STACK

.DATA
    _integer_format_s    db '%d' , 0   ; for scanf
    _integer_format_p    db '%d ', 0   ; for printf
    _char_format_s       db '%c' , 0   ; for scanf
    _char_format_p       db '%c ', 0   ; for printf
    _string_format       db '%s', 0
    _String0             db 10,0
    _String1             db 32,116,111,32,0
    _String2             db 109,118,111,101,32,102,114,111,109,32,0
    _String3             db 110,117,109,98,101,114,32,111,102,32,109,111,118,105,110,103,32,115,116,101,112,32,105,115,32,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   12140

    ; 37        ASG                0               0             i#0
    mov      dword ptr SS:[ebp - 4], 0
    ; 38       READ                0               0             n#3
    lea     eax, SS:[ebp - 16]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ; 39       SETP                0               0             n#3
    push    dword ptr SS:[ebp - 16]
    ; 40  PROC_CALL                0               0               6
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _hanoi6
    add     esp, 8
    ; 41      WRITE                0               0       _string#3
    push    offset  _String3
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 42      WRITE                0               0             i#0
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 43      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8

    mov     esp, ebp
    pop     ebp
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    call    ExitProcess
_main  endp


    ;  1      BEGIN                0               0               6
_hanoi6  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   0

    ; 31       SETP                0               0              97
    push    97
    ; 32       SETP                0               0              98
    push    98
    ; 33       SETP                0               0              99
    push    99
    ; 34       SETP                0               0             n#7
    push    dword ptr SS:[ebp + 12]
    ; 35  PROC_CALL                0               0               8
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 4], eax
    mov     SS:[esp - 8], ebp
    sub     esp, 8
    call    _move8
    add     esp, 24
    ; 36        END                0               0               6
hanoi6_Exit:
    mov     esp, ebp
    pop     ebp
    ret
_hanoi6  endp

    ;  2      BEGIN                0               1               8
_move8  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   4

    ;  3        JNE                1          num#12               0
    mov     eax, 1
    mov     edx, SS:[ebp + 16]
    cmp     eax, edx
    jne     _label0
    ;  4      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  5      WRITE                0               0           src#9
    push    dword ptr SS:[ebp + 28]
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ;  6      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  7      WRITE                0               0          dst#10
    push    dword ptr SS:[ebp + 24]
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ;  8      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  9        ADD              i#0               1             i#0
    mov     ebx, SS:[ebp + 12]
    add      dword ptr SS:[ebx - 4], 1
    ; 10        JMP                0               0               1
    jmp     _label1
    ; 11      LABEL                0               0               0
_label0:
    ; 12       SETP                0               0           src#9
    push    dword ptr SS:[ebp + 28]
    ; 13       SETP                0               0       medium#11
    push    dword ptr SS:[ebp + 20]
    ; 14       SETP                0               0          dst#10
    push    dword ptr SS:[ebp + 24]
    ; 15        SUB           num#12               1         _temp#0
    mov     eax, SS:[ebp + 16]
    sub     eax, 1
    mov      SS:[ebp - 4], eax
    ; 16       SETP                0               0         _temp#0
    push    dword ptr SS:[ebp - 4]
    ; 17  PROC_CALL                0               0               8
    mov     eax, SS:[ebp + 12]
    mov     SS:[esp - 4], eax
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 8], eax
    sub     esp, 8
    call    _move8
    add     esp, 24
    ; 18       SETP                0               0           src#9
    push    dword ptr SS:[ebp + 28]
    ; 19       SETP                0               0          dst#10
    push    dword ptr SS:[ebp + 24]
    ; 20       SETP                0               0               0
    push    0
    ; 21       SETP                0               0               1
    push    1
    ; 22  PROC_CALL                0               0               8
    mov     eax, SS:[ebp + 12]
    mov     SS:[esp - 4], eax
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 8], eax
    sub     esp, 8
    call    _move8
    add     esp, 24
    ; 23       SETP                0               0       medium#11
    push    dword ptr SS:[ebp + 20]
    ; 24       SETP                0               0          dst#10
    push    dword ptr SS:[ebp + 24]
    ; 25       SETP                0               0           src#9
    push    dword ptr SS:[ebp + 28]
    ; 26        SUB           num#12               1         _temp#0
    mov     eax, SS:[ebp + 16]
    sub     eax, 1
    mov      SS:[ebp - 4], eax
    ; 27       SETP                0               0         _temp#0
    push    dword ptr SS:[ebp - 4]
    ; 28  PROC_CALL                0               0               8
    mov     eax, SS:[ebp + 12]
    mov     SS:[esp - 4], eax
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 8], eax
    sub     esp, 8
    call    _move8
    add     esp, 24
    ; 29      LABEL                0               0               1
_label1:
    ; 30        END                0               0               8
move8_Exit:
    mov     esp, ebp
    pop     ebp
    ret
_move8  endp

end start
