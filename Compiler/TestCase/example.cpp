const Symbol = 'c', ten = 10;
var seq : array[10] of integer;
lastChar : char;
procedure Print();
	begin
		write("print calling");
		write(lastChar);
		write(seq[5]);
	end;

begin
	seq[5] := 5;
	write(seq[5]);
	write(1);
	write(('a' + 1) * 6 / 2 + 3 * 8);
	lastChar := 'm';
	Print();
	write(lastChar);
	write(Symbol);
	write(Symbol + ten + seq[5]);
	write("I like writing")
end
.