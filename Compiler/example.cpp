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
			z := x + y
		end;
	begin
		result:= x + y;
		sumF := result
	end;

begin

	i := 10 * 10;
	if i <> 0
		then i:= 0;
	i:= i + 1;
	sum := 0 * i;
	
	for j := i to 10 * 10
	do
	begin
		sequence[j] := j
	end;
	i := 'a';// hehe
	i := 2147483647;
	for j := 100 downto 1
	do
	begin
		write("error \"for sum");
		k := sumF(i, j);
		sum := sumF(sum, j)
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
	write(sum)

end
.