var i, j, k, n, flag : integer;

sequence : array[100] of integer;


begin
	// 计算n以内的，除去[10, 20]之间的质数（n<100）
	read(n);
	k := 0;
	for i := 2 to n do
	begin
		if i >= 10
		then if i <= 20
			then continue;
		for j := 2 to i - 1 do
		begin
			if i / j * j = i
			then break;
		end;
		if j = i	// 是质数
		then 
		begin
			sequence[k] := i;
			k := k + 1;
		end;
	end;
	i := 0;
	while i < k * 2 do
	begin
		if i > 0
		then write(",");
		write(sequence[i]);
		i := i + 1;
		if i >= k
		then break;
	end;
	write("\n")
end
.