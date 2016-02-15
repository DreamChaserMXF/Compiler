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
    sub     esp,   0

    ;  1        JNG                2               1               2
    mov     eax, 2
    cmp     eax, 1
    jng    _label2
    ;  2        JNG                3               2               2
    mov     eax, 3
    cmp     eax, 2
    jng    _label2
    ;  3        JMP                0               0               1
    jmp     _label1
    ;  4      LABEL                0               0               2
_label2:
    ;  5        JMP                0               0               0
    jmp     _label0
    ;  6      LABEL                0               0               1
_label1:
    ;  7      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  8        JMP                0               0               3
    jmp     _label3
    ;  9      LABEL                0               0               0
_label0:
    ; 10      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 11      LABEL                0               0               3
_label3:
    ; 12      WRITE                0               0       _string#0
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
