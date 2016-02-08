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
    _String1             db 10,49,32,60,61,32,105,32,60,61,32,51,32,0
    _String2             db 10,49,32,60,61,32,115,101,113,117,101,110,99,101,91,105,93,32,60,61,32,51,32,0
    _String3             db 10,51,32,60,32,105,32,60,61,32,53,32,0
    _String4             db 10,51,32,60,32,115,101,113,117,101,110,99,101,91,105,93,32,60,61,32,53,32,0
    _String5             db 10,105,32,60,32,49,32,0
    _String6             db 10,105,32,60,32,51,32,0
    _String7             db 10,105,32,62,32,53,32,0
    _String8             db 10,105,32,62,61,32,51,32,0
    _String9             db 10,115,101,113,117,101,110,99,101,91,105,93,32,60,32,49,32,0
    _String10             db 10,115,101,113,117,101,110,99,101,91,105,93,32,62,32,53,32,0
    _String11             db 105,32,61,32,48,0
    _String12             db 105,32,61,32,49,0
    _String13             db 105,32,61,32,50,0
    _String14             db 105,32,61,32,51,0
    _String15             db 105,32,61,32,52,0
    _String16             db 105,32,61,32,53,0
    _String17             db 105,32,61,32,54,0

.CODE

start:

_main  proc far
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    mov     ebp,   esp
    sub     esp,   424

    ;  1       AASG                1               0     sequence#10
    mov     dword ptr SS:[ebp - 24], 1
    ;  2       AASG                2               1     sequence#10
    mov     dword ptr SS:[ebp - 28], 2
    ;  3       AASG                3               2     sequence#10
    mov     dword ptr SS:[ebp - 32], 3
    ;  4       AASG                4               3     sequence#10
    mov     dword ptr SS:[ebp - 36], 4
    ;  5       AASG                5               4     sequence#10
    mov     dword ptr SS:[ebp - 40], 5
    ;  6       AASG                6               5     sequence#10
    mov     dword ptr SS:[ebp - 44], 6
    ;  7        ASG                0               0             i#5
    mov     dword ptr SS:[ebp - 4], 0
    ;  8        JMP                0               0               1
    jmp     _label1
    ;  9      LABEL                0               0               0
_label0:
    ; 10        ADD              i#5               1             i#5
    add     dword ptr SS:[ebp - 4], 1
    ; 11      LABEL                0               0               1
_label1:
    ; 12         JG              i#5               5               2
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 5
    jg     _label2
    ; 13        JNL              i#5               3               3
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 3
    jnl    _label3
    ; 14      WRITE                0               0       _string#6
    push    offset  _String6
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 15      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 16        JMP                0               0               4
    jmp     _label4
    ; 17      LABEL                0               0               3
_label3:
    ; 18      WRITE                0               0       _string#8
    push    offset  _String8
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 19      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 20      LABEL                0               0               4
_label4:
    ; 21        JMP                0               0               0
    jmp     _label0
    ; 22      LABEL                0               0               2
_label2:
    ; 23        ASG                0               0             i#5
    mov     dword ptr SS:[ebp - 4], 0
    ; 24        JMP                0               0               6
    jmp     _label6
    ; 25      LABEL                0               0               5
_label5:
    ; 26        ADD              i#5               1             i#5
    add     dword ptr SS:[ebp - 4], 1
    ; 27      LABEL                0               0               6
_label6:
    ; 28         JG              i#5               5               7
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 5
    jg     _label7
    ; 29        JNG              i#5               5               8
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 5
    jng    _label8
    ; 30      WRITE                0               0       _string#7
    push    offset  _String7
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 31      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 32        JMP                0               0               9
    jmp     _label9
    ; 33      LABEL                0               0               8
_label8:
    ; 34        JNG              i#5               3              10
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 3
    jng    _label10
    ; 35      WRITE                0               0       _string#3
    push    offset  _String3
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 36      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 37        JMP                0               0              11
    jmp     _label11
    ; 38      LABEL                0               0              10
_label10:
    ; 39        JNL              i#5               1              12
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 1
    jnl    _label12
    ; 40      WRITE                0               0       _string#5
    push    offset  _String5
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 41      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 42        JMP                0               0              13
    jmp     _label13
    ; 43      LABEL                0               0              12
_label12:
    ; 44      WRITE                0               0       _string#1
    push    offset  _String1
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 45      WRITE                0               0             i#5
    push    dword ptr SS:[ebp - 4]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 46      LABEL                0               0              13
_label13:
    ; 47      LABEL                0               0              11
_label11:
    ; 48      LABEL                0               0               9
_label9:
    ; 49        JMP                0               0               5
    jmp     _label5
    ; 50      LABEL                0               0               7
_label7:
    ; 51        ASG                0               0             i#5
    mov     dword ptr SS:[ebp - 4], 0
    ; 52        JMP                0               0              15
    jmp     _label15
    ; 53      LABEL                0               0              14
_label14:
    ; 54        ADD              i#5               1             i#5
    add     dword ptr SS:[ebp - 4], 1
    ; 55      LABEL                0               0              15
_label15:
    ; 56         JG              i#5               5              16
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 5
    jg     _label16
    ; 57        ASG      sequence#10             i#5         _temp#0
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    mov     eax,    dword ptr SS:[ecx]
    mov     SS:[ebp - 424], eax
    ; 58        JNG          _temp#0               5              17
    mov     eax,    SS:[ebp - 424]
    cmp     eax, 5
    jng    _label17
    ; 59      WRITE                0               0      _string#10
    push    offset  _String10
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 60      WRITE                0             i#5     sequence#10
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 61        JMP                0               0              18
    jmp     _label18
    ; 62      LABEL                0               0              17
_label17:
    ; 63        ASG      sequence#10             i#5         _temp#0
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    mov     eax,    dword ptr SS:[ecx]
    mov     SS:[ebp - 424], eax
    ; 64        JNG          _temp#0               3              19
    mov     eax,    SS:[ebp - 424]
    cmp     eax, 3
    jng    _label19
    ; 65      WRITE                0               0       _string#4
    push    offset  _String4
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 66      WRITE                0             i#5     sequence#10
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 67        JMP                0               0              20
    jmp     _label20
    ; 68      LABEL                0               0              19
_label19:
    ; 69        ASG      sequence#10             i#5         _temp#0
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    mov     eax,    dword ptr SS:[ecx]
    mov     SS:[ebp - 424], eax
    ; 70        JNL          _temp#0               1              21
    mov     eax,    SS:[ebp - 424]
    cmp     eax, 1
    jnl    _label21
    ; 71      WRITE                0               0       _string#9
    push    offset  _String9
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 72      WRITE                0             i#5     sequence#10
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 73        JMP                0               0              22
    jmp     _label22
    ; 74      LABEL                0               0              21
_label21:
    ; 75      WRITE                0               0       _string#2
    push    offset  _String2
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 76      WRITE                0             i#5     sequence#10
    mov     ecx,    SS:[ebp - 4]
    add     ecx, 6
    shl     ecx,    2
    sub     ecx,    ebp
    neg     ecx
    push    dword ptr SS:[ecx]
    push    offset  _integer_format_p
    call    printf
    add     esp, 8
    ; 77      LABEL                0               0              22
_label22:
    ; 78      LABEL                0               0              20
_label20:
    ; 79      LABEL                0               0              18
_label18:
    ; 80        JMP                0               0              14
    jmp     _label14
    ; 81      LABEL                0               0              16
_label16:
    ; 82      WRITE                0               0       _string#0
    push    offset  _String0
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 83         JE              i#5               0              24
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 0
    je     _label24
    ; 84         JE              i#5               1              25
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 1
    je     _label25
    ; 85         JE              i#5               2              26
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 2
    je     _label26
    ; 86         JE              i#5               3              27
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 3
    je     _label27
    ; 87         JE              i#5               4              28
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 4
    je     _label28
    ; 88         JE              i#5               5              29
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 5
    je     _label29
    ; 89         JE              i#5               6              30
    mov     eax,    SS:[ebp - 4]
    cmp     eax, 6
    je     _label30
    ; 90        JMP                0               0              23
    jmp     _label23
    ; 91      LABEL                0               0              24
_label24:
    ; 92      WRITE                0               0      _string#11
    push    offset  _String11
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 93        JMP                0               0              23
    jmp     _label23
    ; 94      LABEL                0               0              25
_label25:
    ; 95      WRITE                0               0      _string#12
    push    offset  _String12
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 96        JMP                0               0              23
    jmp     _label23
    ; 97      LABEL                0               0              26
_label26:
    ; 98      WRITE                0               0      _string#13
    push    offset  _String13
    push    offset  _string_format
    call    printf
    add     esp, 8
    ; 99        JMP                0               0              23
    jmp     _label23
    ;100      LABEL                0               0              27
_label27:
    ;101      WRITE                0               0      _string#14
    push    offset  _String14
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;102        JMP                0               0              23
    jmp     _label23
    ;103      LABEL                0               0              28
_label28:
    ;104      WRITE                0               0      _string#15
    push    offset  _String15
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;105        JMP                0               0              23
    jmp     _label23
    ;106      LABEL                0               0              29
_label29:
    ;107      WRITE                0               0      _string#16
    push    offset  _String16
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;108        JMP                0               0              23
    jmp     _label23
    ;109      LABEL                0               0              30
_label30:
    ;110      WRITE                0               0      _string#17
    push    offset  _String17
    push    offset  _string_format
    call    printf
    add     esp, 8
    ;111        JMP                0               0              23
    jmp     _label23
    ;112      LABEL                0               0              23
_label23:
    ;113      WRITE                0               0       _string#0
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
