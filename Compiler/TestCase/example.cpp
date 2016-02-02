function func() : integer;
var seq : array[2] of char;
	begin
		seq[1] := 'c';
		write("begin");
		func := seq[1];
		write(1);
	end;

begin
	write(func());
end
.