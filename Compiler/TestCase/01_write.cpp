const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i : integer;
lastChar : char;

sequence : array[100] of integer;

function func(var i : integer) : integer;
	function func1(var j : integer) : char;
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
	i := 13;
	write("\ni * 2 = ", i * 2);
	write("\ni * 16 / i = ", i * 16 / i);
	// �������
	write("\n�������");
	sequence[5] := 138;
	write("\nsequence[5] = ", sequence[5]);
	// �����������ֵ
	write("\n�����������ֵ");
	write(func(100));
	write('\a');
end
.