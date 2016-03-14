.386
.model	flat, stdcall
option	casemap: none

includelib	.\masm32\lib\msvcrt.lib
includelib	.\masm32\lib\kernel32.lib
include	.\masm32\include\msvcrt.inc
include	.\masm32\include\kernel32.inc
printf	PROTO C:	ptr sbyte, :vararg
scanf	PROTO C:	ptr sbyte, :vararg

.STACK

.DATA
	_fmS	db	'%s', 0

	_String0		DB	'asdºÇºÇ', 0

.CODE

start:

_main			proc	far

	push		offset	_String0
	push		offset	_fmS
	call		printf
	add			esp, 8


	push		0
	call		ExitProcess
_main			endp

end start