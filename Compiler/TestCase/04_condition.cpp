const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar : char;
sum : integer;
sequence : array[100] of integer;

begin
	write("\ncompound condition");
	if (1 < 1 && 1 < 2 && 1 < 3
	|| (2 > 1 && (2 = 2 || 2 = 1) && 2 < 3 && (5 > 1))
	|| 3 = 1 && (3 > 2) && 3 = 3)
	then write(" true")
	else write(" false");
	
	i := 1;
	j := 1;
	if (!(i + j) && 1 || 0)
	then write("\n'not' test false")
	else write("\n'not' test true");

	if i + j
	then write("\narithmetic true")
	else write("\narithmetic false");
	
	if i - j
	then write("\narithmetic false")
	else write("\narithmetic true");
	
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
		else write("\ni >= 3 ", i);
		if i > 3 && i < 5
		then write("\n3 < i < 5, i = ", i);
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
	i := 0;
	while i <= 5 do
	begin
		if sequence[i] > 5
		then write("\nsequence[i] > 5 ", sequence[i])
		else if sequence[i] > 3
		then write("\n3 < sequence[i] <= 5 ", sequence[i])
		else if sequence[i] < 1
		then write("\nsequence[i] < 1 ", sequence[i])
		else write("\n1 <= sequence[i] <= 3 ", sequence[i]);
		i := i + 1;
	end;
	write("\n");
	// case语句测试
	case i of
		FALSE, 0, TRUE, 2: write("i <= 2");
		3: write("i = 3");
		4: write("i = 4");
		5: write("i = 5");
		6: write("i = 6");
		7: write("i = 7")
	end;
	write("\n");
	// 单表达式作为条件测试
	if i
	then write("take expression result as a condition")
	else write("condition limited")
	;
	write("\n")
end
.