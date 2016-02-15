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
    _String1             db 10,97,102,116,101,114,32,115,119,97,112,44,32,115,101,113,117,101,110,99,101,91,48,93,32,61,32,0
    _String2             db 115,101,113,117,101,110,99,101,91,48,93,32,61,32,0
    _String3             db 115,101,113,117,101,110,99,101,91,49,93,32,61,32,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   40

    ;  6       AASG                3               0      sequence#0
    mov     dword ptr SS:[ebp - 4], 3
    ;  7       AASG                7               1      sequence#0
    mov     dword ptr SS:[ebp - 8], 7
    ;  8      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  9      WRITE                0               0      sequence#0
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 10      WRITE                0               0       _string#3
    push    offset  _String3
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 11      WRITE                0               1      sequence#0
    push    dword ptr SS:[ebp - 8]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 12    SETREFP                0               0      sequence#0
    lea     eax,    SS:[ebp - 4]
    push    eax
    ; 13    SETREFP                0               1      sequence#0
    lea     eax,    SS:[ebp - 8]
    push    eax
    ; 14  FUNC_CALL                0               0               1
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _func1
    add     esp, 12
    ; 15      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 16      WRITE                0               0      sequence#0
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 17      WRITE                0               0       _string#3
    push    offset  _String3
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 18      WRITE                0               1      sequence#0
    push    dword ptr SS:[ebp - 8]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 19      WRITE                0               0       _string#0
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


    ;  1      BEGIN                0               0               1
_func1  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   4

    ;  2        ASG           i#ref2               0           tmp#4
    mov     ebx,    SS:[ebp + 16]
    mov     eax,    SS:[ebx]
    mov     SS:[ebp - 4], eax
    ;  3        ASG           j#ref3               0          i#ref2
    mov     ebx,    SS:[ebp + 12]
    mov     eax,    SS:[ebx]
    mov     ebx,    SS:[ebp + 16]
    mov     SS:[ebx], eax
    ;  4        ASG            tmp#4               0          j#ref3
    mov     eax,    SS:[ebp - 4]
    mov     ebx,    SS:[ebp + 12]
    mov     SS:[ebx], eax
    ;  5        END                0               0               1
func1_Exit:
    mov     esp,    ebp
    pop     ebp
    ret
_func1  endp

end start
