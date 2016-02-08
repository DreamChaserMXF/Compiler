const ARRLEN = + 100, NEGATIVE = -1, TRUE = 1, FALSE = 0, ENDCHAR = 'a';

var i, j, k : integer;
lastChar : char;
sum : integer;
sequence : array[100] of integer;

function func(i : integer) : integer;
begin
	func := i + 1;
end;

begin
	sequence[0] := 1;
	sequence[1] := 2;
	sequence[2] := 3;
	sequence[3] := 4;
	sequence[4] := 5;
	sequence[5] := 6;

	// 输入 1 2 3  输出 1 2 3 2 3 4
	read(i, j, k);
	write(i);
	write(j);
	write(k);
	write(func(i));
	write(func(j));
	write(func(k));
	write("\n");
	// NEG测试	预期结果：-1 -1 -4 -2 -5
	write(-i);
	write(-j+i);
	write(-k+(-i));
	write(-sequence[1]);
	write(-(sequence[1]+sequence[2]));
	write("\n");
	// ADD测试  预期结果：3 3 6 6 6 6 3 -7
	write(i+j);
	write(j+i);
	write(i+j+k);
	write(((i+j)+k));
	write(i+(j+k));
	write(k+j+i);
	write(i+sequence[1]);
	write(-(sequence[1]+j+sequence[2]));
	write("\n");
	// SUB测试  预期结果：-1 1 -4 -4 2 0 -1, -1, 5
	write(i-j);
	write(j-i);
	write(i-j-k);
	write((i-j)-k);
	write(i-(j-k));
	write(k-(j+i));
	write((k-i)-(j+i));
	write(i-sequence[1]);
	write(k-(sequence[1]-j-(sequence[2]-i)));
	write("\n");
	// IMUL测试  预期结果：2 3 6 7 9 -1 2 4 6 54
	write(i*j);
	write(i*k);
	write(j*k);
	write(i+j*k);
	write((i+j)*k);
	write(i*(j-k));
	write(i*sequence[1]);
	write(j*sequence[1]);
	write(k*sequence[1]);
	write((i+j)*(sequence[1]*sequence[2])*k);
	write("\n");
	// IDIV测试  预期结果：0 0 0 2 3 1 1 1 -1 0 1 1 2 -6
	write(i/j);
	write(i/k);
	write(j/k);
	write(j/i);
	write(k/i);
	write(k/j);
	write(i+j/k);
	write((i+j)/k);
	write(i/(j-k));
	write(i/sequence[1]);
	write(j/sequence[1]);
	write(k/sequence[1]);
	write((k+1)/sequence[1]);
	write((i+j)*(sequence[1]*sequence[2])/(0-k));
	write("\n");	
end
.