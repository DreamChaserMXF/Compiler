var i, j, k, n, result : integer;

// ��iȡ��
procedure neg1(var i : integer);
begin
	i := -i;
end;

// ��iȡ��
procedure neg2(var i : integer);
	procedure func();
	begin
		i := -i;
	end;
begin
	 func();
end;

// ����i��j
procedure swap1(var i, j : integer);
var tmp : integer;
begin
	tmp := i;
	i := j;
	j := tmp;
end;

// ����i��j
procedure swap2(var i, j : integer);
var tmp : integer;
	procedure func();
	begin
		tmp := i;
		i := j;
		j := tmp;
	end;
begin
	 func();
end;

// ��i��ֵ
procedure assign1(var k : integer);
begin
	 k := 5;
end;

// ��i��ֵ
procedure assign2(var k : integer);
var tmp : integer;
	procedure func();
	begin
		k := 10;
	end;
begin
	 func();
end;

begin
	i := 1;
	j := 2;
	write("i = ", i);
	write("j = ", j);
	// ȡ������
	neg1(i);
	write("\nafter neg: i = ", i);
	neg2(i);
	write("\nafter neg: i = ", i);
	// ��������
	swap1(i, j);
	write("\nafter swap: i = ", i);
	write("j = ", j);
	swap2(i, j);
	write("\nafter swap: i = ", i);
	write("j = ", j);
	// ��ֵ����
	assign1(i);
	write("\nafter assigning: i = ", i);
	assign2(i);
	write("\nafter assigning: i = ", i);
	write("\n")
end
.