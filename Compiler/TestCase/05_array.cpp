var i : integer;

sequence : array[100] of integer;

procedure func();
var sequence1 : array[100] of integer;
//i : integer;
begin
	// 初始化
	sequence1[0] := 0;
	sequence1[1] := 1;
	sequence1[2] := 2;
	sequence1[3] := 3;
	sequence1[4] := 4;
	sequence1[5] := 5;
	
	sequence[0] := 0;
	sequence[1] := 1;
	sequence[2] := 2;
	sequence[3] := 3;
	sequence[4] := 4;
	sequence[5] := 5;

	// 输出
	for i := 0 to 5 do
	begin
		write(sequence[i]);
		write(sequence1[i]);
		write("\n");
	end;
	
	//赋值
	for i := 0 to 5 do
	begin
		sequence[i] := i * 10;
		sequence1[i] := i * 100;
	end;
	// i := 5;
	// sequence1[i] := i;
	// write(sequence1[i]);
	// sequence[i] := i;
	// write(sequence[i]);

	// 输出
	for i := 0 to 5 do
	begin
		write(sequence[i]);
		write(sequence1[i]);
		write("\n");
	end;
	
	i := sequence[5];
	write(i);
	i := sequence1[5];
	write(i);
	write("\n")
end;

begin
	func();
end
.