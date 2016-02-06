var i, j, k, n, result : integer;

// 计算阶乘
function factorial(var n : integer) : integer;
begin
	if n <= 1
	then factorial := 1
	else 
	begin
		// 测试点：
		// 2. 表达式中对函数的调用
		// 3. 函数的递归调用
		result := n * factorial(n - 1);
		factorial := result;
	end
end;

begin
	write("input a value n, output the factorial of n\n");
	read(n);
	// 测试点：1. 普通语句中对函数的调用
	factorial(n);
	n := result;
	if n > 0
	then 
		write("n! = ", n)
	else
		write("value too large")
	;
	write("\n")
end
.