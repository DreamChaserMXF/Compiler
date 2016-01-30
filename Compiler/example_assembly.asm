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

main:  proc far
