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
    _String1             db 49,0
    _String2             db 98,117,103,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   12

    ;  1        ASG                1               0             a#0
    mov     dword ptr SS:[ebp - 4], 1
    ;  2        ASG               -1               0             b#1
    mov     dword ptr SS:[ebp - 8], -1
    ;  3        ADD              a#0             b#1         _temp#0
    mov     eax,    SS:[ebp - 4]
    add     eax,    SS:[ebp - 8]
    mov     SS:[ebp - 12], eax
    ;  4         JE          _temp#0               0               0
    mov     eax,    SS:[ebp - 12]
    cmp     eax, 0
    je     _label0
    ;  5      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  6        JMP                0               0               1
    jmp     _label1
    ;  7      LABEL                0               0               0
_label0:
    ;  8      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  9      LABEL                0               0               1
_label1:
    ; 10      WRITE                0               0       _string#0
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


end start
