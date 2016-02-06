var sequence : array[10] of integer;
function func(var i, j : integer) : integer;
var tmp : integer;
begin
	tmp := i;
	i := j;
	j := tmp;
end;

begin
	sequence[0] := 3;
	sequence[1] := 7;
	write("sequence[0] = ", sequence[0]);
	write("sequence[1] = ", sequence[1]);
	func(sequence[0], sequence[1]);
	write("\nafter swap, sequence[0] = ", sequence[0]);
	write("sequence[1] = ", sequence[1]);
	write("\n");
end
.