var i, j, k, n, flag : integer;

// ¼ÆËã½×³Ë
function factorial(n : integer) : integer;
begin
	if n <= 1
	then factorial := 1
	else factorial := n * factorial(n - 1);
end;

begin
	write("input a value n, output the factorial of n\n");
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