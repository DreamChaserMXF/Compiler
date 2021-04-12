var i, j, k, n, res, flag : integer;

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
	res := factorial(n);
	if n > 0 then
    begin
        write("n = ", n);
        write("\n");
		write("n! = ", res);
    end
	else
		write("value too large")
	;
	write("\n")
end
.