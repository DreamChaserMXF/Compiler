const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar : char;
sum : integer;
sequence : array[100] of integer;

begin
	sequence[0] := 1;
	sequence[1] := 2;
	sequence[2] := 3;
	sequence[3] := 4;
	sequence[4] := 5;
	sequence[5] := 6;
	// 简单条件语句测试
	for i := 0 to 5 do
	begin
		if i < 3
		then write("\ni < 3 ", i)
		else write("\ni >= 3 ", i)
	end;
	// 嵌套条件语句测试
	for i := 0 to 5 do
	begin
		if i > 5
		then write("\ni > 5 ", i)
		else if i > 3
		then write("\n3 < i <= 5 ", i)
		else if i < 1
		then write("\ni < 1 ", i)
		else write("\n1 <= i <= 3 ", i)
	end;
	// 数组元素的条件语句测试
	for i := 0 to 5 do
	begin
		if sequence[i] > 5
		then write("\nsequence[i] > 5 ", sequence[i])
		else if sequence[i] > 3
		then write("\n3 < sequence[i] <= 5 ", sequence[i])
		else if sequence[i] < 1
		then write("\nsequence[i] < 1 ", sequence[i])
		else write("\n1 <= sequence[i] <= 3 ", sequence[i])
	end;
	write("\n");
	// case语句测试
	case i of
		0: write("i = 0");
		1: write("i = 1");
		2: write("i = 2");
		3: write("i = 3");
		4: write("i = 4");
		5: write("i = 5");
		6: write("i = 6")
	end;
	write("\n");
end
.