const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar : char;
sum : integer;
sequence : array[100] of integer;


procedure sumP(x, y: char; var result : integer);
	var tempSumP : integer;
	begin
		result:= - x + y
	end;

function sumF(x, y : integer) : integer;
	var result : integer;
	procedure sumFF(x, y : integer);
		var z : integer;
		begin
			z := x + y + 1 + x + y
		end;
	begin
		result:= x + y;
		sumF := result;
		sumFF(x, y)
	end;

begin

	i := 10 * 10;
	write("i = ", i);
	if i <> 0
		then i:= 0;
	write("i = ", i);
	i:= i + 1;
	write("i = ", i);
	sum := 0 * i;
	write("sum = ", sum);
	
	if 0 = sum
		then sum := 1
		else sum := 0
	; // 表示if语句的结束
	write("sum = ", sum);
	
	// 空语句块测试
	begin
	end
	;
	
	i:= 1;
	i := (1*2) + (3*4) / (5*6+7*8);
	write("i = ", i);
	sequence[0] := 0;
	for j := i to 10 * 10 - 1
	do
	begin
		sequence[j] := j + sequence[j - 1];
		write("sequence[", j);
		write("] = ", j);
		if 0 = sequence[j]
		then break;
		if 1 = sequence[j]
		then
		else continue
	end;
	sequence[1] := -sequence[1];
	write("sequence[1] = ", sequence[1]);
	while i < 10
	do
	begin
		i := i + 1;
		if 3 = i
		then 
		begin
			i := i + 2;
		end
		else if 5 = i
		then break
		else continue
	end	;
	
	sequence[sequence[0]] := sequence[sequence[1] + sequence[2] + sequence[1+2]];
	i := 'a';// hehe
	i := 2147483647;
	i := ARRLEN;
	for j := 100 downto 1
	do
	begin
		write("error for \"sum\"");
		k := sumF(i, j + i);
		sum := sumF(sum, j - i)
	end;
/*
	begin
*/	
	case sum * 2 + 1 of
		1 : write("error for sum");
		10101: write("right for sum");
		520 : write("error for sum");
		ARRLEN : write("error for sum")
	end;

	read(sum);
	sumP(ENDCHAR, lastChar, sum);	
	sumP(ENDCHAR, 'c', sequence[3+5]);	
	write(sum);
	write(sequence[j+1+2])

end
.