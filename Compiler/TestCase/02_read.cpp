const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar, space : char;
seq_int : array[10] of integer;
seq_char : array[10] of char;
begin
	write("��������������: ");
	read(i, j, k);
	write(i);
	write(j);
	write(k);
	write("\n������һ���ַ�: ");
	read(space);
	read(lastChar);
	write(lastChar);
	write("\n���������������������ַ�(�ÿո����): ");
	read(seq_int[0], seq_int[1], space, seq_char[0], space, seq_char[1], space);
	write(seq_char[1]);write(seq_char[0]);
	write(seq_int[1]);write(seq_int[0]);
	write("\n");
end
.