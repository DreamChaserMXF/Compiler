var i, j, k, n, result, final_res : integer;

// ����׳�
function factorial(n : integer) : integer;
begin
	if n <= 1
	then factorial := 1
	else 
	begin
		// ���Ե㣺
		// 2. ���ʽ�жԺ����ĵ���
		// 3. �����ĵݹ����
		result := n * factorial(n - 1);
		factorial := result;
	end
end;

begin
	write("input a value n, output the factorial of n\n");
	read(n);
	// ���Ե㣺1. ��ͨ����жԺ����ĵ���
	final_res := factorial(n);
	if n > 0 then
	begin
        write("n = ", n);
        write("\n");
		write("n! = ", final_res);
    end
	else
		write("value too large")
	;
	write("\n")
end
.