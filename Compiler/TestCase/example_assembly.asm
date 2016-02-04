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
    _String0             db 10,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, esp
    push    eax
    push    offset _integer_format_p
    call    printf
    add     esp, 8
    push    ebp
    mov     ebp,   esp
    sub     esp,   404
    mov     eax, ebp
    push    eax
    push    offset _integer_format_p
    call    printf
    add     esp, 8

    ; 54  PROC_CALL                0               0               2
    mov     SS:[esp - 4], ebp
    sub     esp, 4
    call    _func2
    add     esp, 4

    mov     eax, ebp
    push    eax
    push    offset _integer_format_p
    call    printf
    add     esp, 8
    mov     esp, ebp
    pop     ebp
    mov     eax, esp
    push    eax
    push    offset _integer_format_p
    call    printf
    add     esp, 8
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    call    ExitProcess
_main  endp


    ;  1      BEGIN                0               1               2
_func2  proc near
    push    ebp
    mov     ebp,   esp
    sub     esp,   404

    ;  2       AASG                0               0     sequence1#3
    mov      dword ptr SS:[ebp - 4], 0
    ;  3       AASG                1               1     sequence1#3
    mov      dword ptr SS:[ebp - 8], 1
    ;  4       AASG                2               2     sequence1#3
    mov      dword ptr SS:[ebp - 12], 2
    ;  5       AASG                3               3     sequence1#3
    mov      dword ptr SS:[ebp - 16], 3
    ;  6       AASG                4               4     sequence1#3
    mov      dword ptr SS:[ebp - 20], 4
    ;  7       AASG                5               5     sequence1#3
    mov      dword ptr SS:[ebp - 24], 5
    ;  8       AASG                0               0      sequence#1
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 8], 0
    ;  9       AASG                1               1      sequence#1
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 12], 1
    ; 10       AASG                2               2      sequence#1
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 16], 2
    ; 11       AASG                3               3      sequence#1
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 20], 3
    ; 12       AASG                4               4      sequence#1
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 24], 4
    ; 13       AASG                5               5      sequence#1
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 28], 5
    ; 14        ASG                0               0             i#0
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 4], 0
    ; 15        JMP                0               0               1
    jmp     _label1
    ; 16      LABEL                0               0               0
_label0:
    ; 17        ADD              i#0               1             i#0
    mov     ebx, SS:[ebp + 8]
    add      dword ptr SS:[ebx - 4], 1
    ; 18      LABEL                0               0               1
_label1:
    ; 19         JG              i#0               5               2
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 4]
    mov     edx, 5
    cmp     eax, edx
    jg      _label2
    ; 20      WRITE                0             i#0      sequence#1
    mov     ebx, dword ptr SS:[ebp + 8]
    mov     ebx, SS:[ebp + 8]
    mov     ecx, SS:[ebx - 4]
    add     ecx, 2
    shl     ecx, 2
    sub     ecx, ebx
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 21      WRITE                0             i#0     sequence1#3
    mov     ebx, SS:[ebp + 8]
    mov     ecx, SS:[ebx - 4]
    add     ecx, 1
    shl     ecx, 2
    sub     ecx, ebp
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 22      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 23        JMP                0               0               0
    jmp     _label0
    ; 24      LABEL                0               0               2
_label2:
    ; 25        ASG                0               0             i#0
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 4], 0
    ; 26        JMP                0               0               4
    jmp     _label4
    ; 27      LABEL                0               0               3
_label3:
    ; 28        ADD              i#0               1             i#0
    mov     ebx, SS:[ebp + 8]
    add      dword ptr SS:[ebx - 4], 1
    ; 29      LABEL                0               0               4
_label4:
    ; 30         JG              i#0               5               5
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 4]
    mov     edx, 5
    cmp     eax, edx
    jg      _label5
    ; 31        MUL              i#0              10         _temp#0
    mov     eax, 10
    mov     ebx, dword ptr SS:[ebp + 8]
    imul    dword ptr SS:[ebx - 4]
    mov      SS:[ebp - 404], eax
    ; 32       AASG          _temp#0             i#0      sequence#1
    mov     eax, SS:[ebp - 404]
    mov     ebx, SS:[ebp + 8]
    mov     ebx, SS:[ebp + 8]
    mov     ecx, SS:[ebx - 4]
    add     ecx, 2
    shl     ecx, 2
    sub     ecx, ebx
    neg     ecx
    mov     SS:[ecx], eax
    ; 33        MUL              i#0             100         _temp#0
    mov     eax, 100
    mov     ebx, dword ptr SS:[ebp + 8]
    imul    dword ptr SS:[ebx - 4]
    mov      SS:[ebp - 404], eax
    ; 34       AASG          _temp#0             i#0     sequence1#3
    mov     eax, SS:[ebp - 404]
    mov     ebx, SS:[ebp + 8]
    mov     ecx, SS:[ebx - 4]
    add     ecx, 1
    shl     ecx, 2
    sub     ecx, ebp
    neg     ecx
    mov     dword ptr SS:[ecx], eax
    ; 35        JMP                0               0               3
    jmp     _label3
    ; 36      LABEL                0               0               5
_label5:
    ; 37        ASG                0               0             i#0
    mov     ebx, SS:[ebp + 8]
    mov      dword ptr SS:[ebx - 4], 0
    ; 38        JMP                0               0               7
    jmp     _label7
    ; 39      LABEL                0               0               6
_label6:
    ; 40        ADD              i#0               1             i#0
    mov     ebx, SS:[ebp + 8]
    add      dword ptr SS:[ebx - 4], 1
    ; 41      LABEL                0               0               7
_label7:
    ; 42         JG              i#0               5               8
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 4]
    mov     edx, 5
    cmp     eax, edx
    jg      _label8
    ; 43      WRITE                0             i#0      sequence#1
    mov     ebx, dword ptr SS:[ebp + 8]
    mov     ebx, SS:[ebp + 8]
    mov     ecx, SS:[ebx - 4]
    add     ecx, 2
    shl     ecx, 2
    sub     ecx, ebx
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 44      WRITE                0             i#0     sequence1#3
    mov     ebx, SS:[ebp + 8]
    mov     ecx, SS:[ebx - 4]
    add     ecx, 1
    shl     ecx, 2
    sub     ecx, ebp
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 45      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 46        JMP                0               0               6
    jmp     _label6
    ; 47      LABEL                0               0               8
_label8:
    ; 48        ASG       sequence#1               5             i#0
    mov     ebx, SS:[ebp + 8]
    mov     eax, SS:[ebx - 28]
    mov     ebx, SS:[ebp + 8]
    mov      SS:[ebx - 4], eax
    ; 49      WRITE                0               0             i#0
    mov     ebx, dword ptr SS:[ebp + 8]
    push    dword ptr SS:[ebx - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 50        ASG      sequence1#3               5             i#0
    mov     eax, SS:[ebp - 24]
    mov     ebx, SS:[ebp + 8]
    mov      SS:[ebx - 4], eax
    ; 51      WRITE                0               0             i#0
    mov     ebx, dword ptr SS:[ebp + 8]
    push    dword ptr SS:[ebx - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 52      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 53        END                0               0               2
func2_Exit:
    mov     esp, ebp
    pop     ebp
    ret
_func2  endp

end start
