TITLE MXF_AssemblyCode

.386
.model flat, stdcall
option casemap: none

includelib .\masm32\lib\msvcrt.lib
includelib .\masm32\lib\kernel32.lib
includelib .\masm32\include\msvcrt.inc
includelib .\masm32\include\kernel32.inc
printf PROTO C: ptr sbyte, :vararg
scanf  PROTO C: ptr sbyte, :vararg

.STACK

.DATA
    _integer_format    db '%d ', 0
    _char_format       db '%c ', 0
    _str_format        db '%s ', 0

.CODE

.start:

_main:  proc far

    push    0
    call    ExitProcess
_main:  endp


_sumP      proc near

_sumP      endp

_sumF      proc near

_sumF      endp

_sumFF     proc near

_sumFF     endp

.end start
