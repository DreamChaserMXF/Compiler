const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar : char;
sum : integer;
sequence : array[100] of integer;

function func(i : integer) : integer;
	function func1(j : integer) : char;
	begin
		func1 := func(j) + 10;
	end;
begin
	if i = 101
	then func := 102
	else func := func1(i + 1);
end;

begin
	i := 1;
	// ����ַ���
	write("\n����ַ���\n");
	write("123\n");
	write(10);
	// // ���������
	write("\n���������\n");
	write('c');
	// �������
	write("\n�������\n");
	write(ARRLEN);
	write("\n", ENDCHAR);
	// ������ʽ
	write("\n������ʽ");
	lastChar := '1';
	write("\n", lastChar);
	write("\n", lastChar+1);
	// �����������ֵ
	write("\n�����������ֵ");
	write(func(100));
	write('\a');
end
.