var i, j, k, n, flag : integer;

sequence : array[3030] of integer;

procedure hanoi(n : integer);
	procedure move(src, dst, medium : char; num : integer);
	begin
		if 1 = num then
		begin
			write("mvoe from ", src);
			write(" to ", dst);
			write("\n");
			i := i + 1;
		end
		else
		begin
			move(src, medium, dst, num - 1);
			move(src, dst, '\0', 1);
			move(medium, dst, src, num - 1);
		end
	end;
begin
	move('a', 'b', 'c', n)
	// move('a', 'c', 'b', n - 1);
	// move('a', 'b', '\0', 1);
	// move('c', 'b', 'a', n - 1);
end;

begin
	i := 0;
	write("ººÅµËşÓÎÏ·£¬ÇëÊäÈëaËşµÄÔ²ÅÌÊıÁ¿n\n");
	write("n: ");
	read(n);
	hanoi(n);
	write("number of moving step is ", i);
	write("\n")
end
.