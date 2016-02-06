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
    _String1             db 105,110,112,117,116,32,97,32,118,97,108,117,101,32,110,44,32,111,117,116,112,117,116,32,116,104,101,32,102,97,99,116,111,114,105,97,108,32,111,102,32,110,10,0
    _String2             db 110,33,32,61,32,0
    _String3             db 118,97,108,117,101,32,116,111,111,32,108,97,114,103,101,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   20

    ; 14      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 15       READ                0               0             n#3
    lea     eax, SS:[ebp - 16]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ; 16       SETP                0               0             n#3
    push    dword ptr SS:[ebp - 16]
    ; 17  FUNC_CALL                0               0               5
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _factorial5
    add     esp, 8
    ; 18        ASG         result#4               0             n#3
    mov     eax, SS:[ebp - 20]
    mov      SS:[ebp - 16], eax
    ; 19        JNG              n#3               0               2
    mov     eax, SS:[ebp - 16]
    mov     edx, 0
    cmp     eax, edx
    jng     _label2
    ; 20      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 21      WRITE                0               0             n#3
    push    dword ptr SS:[ebp - 16]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 22        JMP                0               0               3
    jmp     _label3
    ; 23      LABEL                0               0               2
_label2:
    ; 24      WRITE                0               0       _string#3
    push    offset  _String3
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 25      LABEL                0               0               3
_label3:
    ; 26      WRITE                0               0       _string#0
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


    ;  1      BEGIN                0               1               5
_factorial5  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   4

    ;  2         JG              n#6               1               0
    mov     eax, SS:[ebp + 12]
    mov     edx, 1
    cmp     eax, edx
    jg      _label0
    ;  3        RET                0               0               1
    mov     eax, 1
    jmp     factorial5_Exit
    ;  4        JMP                0               0               1
    jmp     _label1
    ;  5      LABEL                0               0               0
_label0:
    ;  6        SUB              n#6               1         _temp#0
    mov     eax, SS:[ebp + 12]
    sub     eax, 1
    mov      SS:[ebp - 4], eax
    ;  7       SETP                0               0         _temp#0
    push    dword ptr SS:[ebp - 4]
    ;  8  FUNC_CALL                0               0               5
    mov     eax, SS:[ebp + 8]
    mov     SS:[esp - 4], eax
    sub     esp, 4
    call    _factorial5
    add     esp, 8
    ;  9      STORE                0               0         _temp#0
    mov      SS:[ebp - 4], eax
    ; 10        MUL              n#6         _temp#0        result#4
    mov     eax, SS:[ebp - 4]
    imul    dword ptr SS:[ebp + 12]
    mov     ebx, SS:[ebp + 8]
    mov      SS:[ebx - 20], eax
    ; 11        RET                0               0        result#4
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 20]
    jmp     factorial5_Exit
    ; 12      LABEL                0               0               1
_label1:
    ; 13        END                0               0               5
factorial5_Exit:
    mov     esp, ebp
    pop     ebp
    ret
_factorial5  endp

end start
