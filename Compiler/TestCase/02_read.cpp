const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar, space : char;
seq_int : array[10] of integer;
seq_char : array[10] of char;
begin
	write("请输入三个整数: ");
	read(i, j, k);
	write(i);
	write(j);
	write(k);
	write("\n请输入一个字符: ");
	read(space);
	read(lastChar);
	write(lastChar);
	write("\n请输入两个整数和两个字符(用空格隔开): ");
	read(seq_int[0], seq_int[1], space, seq_char[0], space, seq_char[1], space);
	write(seq_char[1]);write(seq_char[0]);
	write(seq_int[1]);write(seq_int[0]);
	write("\n");
end
.