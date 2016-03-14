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
    _String0             db 10,-24,-81,-73,-24,-66,-109,-27,-123,-91,-28,-72,-128,-28,-72,-86,-27,-83,-105,-25,-84,-90,0
    _String1             db 10,-24,-81,-73,-24,-66,-109,-27,-123,-91,-28,-72,-92,-28,-72,-86,-26,-107,-76,-26,-107,-80,-27,-110,-116,-28,-72,-92,-28,-72,-86,-27,-83
	db -105
						 db -25,-84,-90,0
    _String2             db -24,-81,-73,-24,-66,-109,-27,-123,-91,-28,-72,-119,-28,-72,-86,-26,-107,-76,-26,-107,-80,58,32,32,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   100

    ;  1      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  2       READ                0               0             i#5
    lea     eax,    SS:[ebp - 4]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ;  3       READ                0               0             j#6
    lea     eax,    SS:[ebp - 8]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ;  4       READ                0               0             k#7
    lea     eax,    SS:[ebp - 12]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ;  5      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ;  6      WRITE                0               0             j#6
    push    dword ptr SS:[ebp - 8]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ;  7      WRITE                0               0             k#7
    push    dword ptr SS:[ebp - 12]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ;  8      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;  9       READ                0               0         space#9
    lea     eax,    SS:[ebp - 20]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 10       READ                0               0      lastChar#8
    lea     eax,    SS:[ebp - 16]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 11      WRITE                0               0      lastChar#8
    push    dword ptr SS:[ebp - 16]
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ; 12      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 13       READ                0               0      seq_int#10
    lea     eax,    SS:[ebp - 24]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ; 14       READ                0               1      seq_int#10
    lea     eax,    SS:[ebp - 28]
    push    eax
    push    offset  _integer_format_s
    call    scanf
    add     esp, 8
    ; 15       READ                0               0         space#9
    lea     eax,    SS:[ebp - 20]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 16       READ                0               0     seq_char#11
    lea     eax,    SS:[ebp - 64]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 17       READ                0               0         space#9
    lea     eax,    SS:[ebp - 20]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 18       READ                0               1     seq_char#11
    lea     eax,    SS:[ebp - 68]
    push    eax
    push    offset  _char_format_s
    call    scanf
    add     esp, 8
    ; 19      WRITE                0               1     seq_char#11
    push    dword ptr SS:[ebp - 68]
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ; 20      WRITE                0               0     seq_char#11
    push    dword ptr SS:[ebp - 64]
    push    offset  _char_format_p
    call    printf
    add     esp, 8
    ; 21      WRITE                0               1      seq_int#10
    push    dword ptr SS:[ebp - 28]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 22      WRITE                0               0      seq_int#10
    push    dword ptr SS:[ebp - 24]
    push    offset  _integer_format_p
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
