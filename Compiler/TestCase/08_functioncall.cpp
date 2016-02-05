var i, j, k, n, flag : integer;

sequence : array[2000] of integer;

// ¼ÆËã½×³Ë
function factorial(var n : integer) : integer;
begin
	if n <= 1
	then factorial := 1
	else factorial := n * factorial(n - 1);
end;

begin
	read(n);
	n := factorial(n);
	if n > 0
	then 
		write("n! = ", n)
	else
		write("value too large")
	;
	write("\n")
end
.