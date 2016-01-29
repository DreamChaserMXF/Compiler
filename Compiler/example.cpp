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
	if i <> 0
		then i:= 0;
	i:= i + 1;
	sum := 0 * i;
	
	i:= 1;
	sequence[0] := 0;
	for j := i to 10 * 10 - 1
	do
	begin
		sequence[j] := j + sequence[j - 1]
	end;
	sequence[1] := -sequence[1];
	
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
	case sum of
		0 : write("error for sum");
		5050: write("right for sum")
	end;

	read(sum);
	sumP(ENDCHAR, lastChar, sum);	
	sumP(ENDCHAR, 'c', sequence[3+5]);	
	write(sum);
	write(sequence[j+1+2])

end
.