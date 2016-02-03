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
	// 输出字符串
	write("\n输出字符串\n");
	write("123\n");
	write(10);
	// // 输出立即数
	write("\n输出立即数\n");
	write('c');
	// 输出常量
	write("\n输出常量\n");
	write(ARRLEN);
	write("\n", ENDCHAR);
	// 输出表达式
	write("\n输出表达式");
	lastChar := '1';
	write("\n", lastChar);
	write("\n", lastChar+1);
	i := 13;
	write("\ni * 2 = ", i * 2);
	write("\ni * 16 / i = ", i * 16 / i);
	// 输出数组
	write("\n输出数组");
	sequence[5] := 138;
	write("\nsequence[5] = ", sequence[5]);
	// 输出函数返回值
	write("\n输出函数返回值");
	write(func(100));
	write('\a');
end
.