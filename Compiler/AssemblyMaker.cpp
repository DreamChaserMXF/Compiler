#include "AssemblyMaker.h"
#include "Print.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <assert.h>
using std::cout;
using std::endl;

AssemblyMaker::AssemblyMaker(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, const vector<string> &stringtable) throw()
	: quaternarytable_(quaternarytable), tokentable_(tokentable), stringtable_(stringtable), assemble_buffer()
{
}

void AssemblyMaker::Print(std::ostream &out) const throw()
{
	out << assemble_buffer.str();
}
bool AssemblyMaker::Print(const string &filename) const throw()
{
	std::ofstream out(filename);
	if(!out.is_open())
	{
		cout << "Cannot open file " << filename << endl;
		return false;
	}
	Print(out);
	return true;
}

bool AssemblyMaker::Assemble() throw()
{
	assemble_buffer.str("");
	Head();
	StackSegment();
	DataSegment();
	CodeBeginSegment();
	MainFunction();
	// traverse the tokentable
	for(TokenTable::const_iterator c_iter = tokentable_.begin();
		c_iter != tokentable_.end(); ++c_iter)
	{
		if(TokenTableItem::PROCEDURE == c_iter->itemtype_
			|| TokenTableItem::FUNCTION == c_iter->itemtype_)
		{
			OtherFunction(c_iter);
		}
	}
	EndStatement();
	return true;
}

void AssemblyMaker::Head() throw()
{
	// 标题
	assemble_buffer << "TITLE MXF_AssemblyCode\n";
	
	// 指令模式（32位），寻址模式（平坦寻址），大小写模式（敏感）
	assemble_buffer << "\n.386";
	assemble_buffer << "\n.model flat, stdcall";
	assemble_buffer << "\noption casemap: none\n";
	// 包含的库
	assemble_buffer << "\nincludelib .\\masm32\\lib\\msvcrt.lib      ; for printf & scanf linking";
	assemble_buffer << "\nincludelib .\\masm32\\lib\\kernel32.lib    ; for ExitProcess linking";
	assemble_buffer << "\ninclude .\\masm32\\include\\msvcrt.inc     ; 似乎没啥用，应该是printf和scanf的声明文件";
	assemble_buffer << "\ninclude .\\masm32\\include\\kernel32.inc   ; for ExitProcess";
	// 输入输出函数调用
	assemble_buffer << "\nprintf PROTO C: ptr sbyte, :vararg";
	assemble_buffer << "\nscanf  PROTO C: ptr sbyte, :vararg";
	assemble_buffer << endl;
}
void AssemblyMaker::StackSegment() throw()
{
	assemble_buffer << "\n.STACK" << endl;
}


void AssemblyMaker::DataSegment() throw()
{
	assemble_buffer << "\n.DATA";
	assemble_buffer << "\n    _integer_format_s    db '%d' , 0   ; for scanf";
	assemble_buffer << "\n    _integer_format_p    db '%d ', 0   ; for printf";
	assemble_buffer << "\n    _char_format_s       db '%c' , 0   ; for scanf";
	assemble_buffer << "\n    _char_format_p       db '%c ', 0   ; for printf";
	assemble_buffer << "\n    _string_format       db '%s', 0";
	for(size_t i = 0; i < stringtable_.size(); ++i)
	{
		assemble_buffer << "\n    _String" << i << "             db ";
		for(string::const_iterator iter = stringtable_[i].begin();
			iter != stringtable_[i].end(); ++iter)
		{
			// 每48个整数换一行（如果设为极限，即49个数换行，但最后还有一个0，加上之后可能就超了
			if(iter != stringtable_[i].begin() && distance(iter, stringtable_[i].begin()) % 47 == 0)	
			{
				assemble_buffer << static_cast<int>(*iter) << "\n	                     db ";
			}
			else
			{
				assemble_buffer << static_cast<int>(*iter) << ",";
			}
		}
		assemble_buffer << "0";
	}
	assemble_buffer << endl;
}
void AssemblyMaker::CodeBeginSegment() throw()
{
	assemble_buffer << "\n.CODE" << endl;
}
void AssemblyMaker::MainFunction() throw()
{
	// 主函数声明
	assemble_buffer << "\nstart:\n";				// 开始位置
	assemble_buffer << "\n_main  proc far";		// 跳转方式
	// 主函数头
	// 一. 找出主函数中局部变量与临时变量的存储空间
	int var_space = tokentable_.GetVariableSpace(tokentable_.begin());;
	int temp_space = quaternarytable_.front().src2_;
	// 二. 保存寄存器并构造运行栈
	// 会用到 eax ebx edx ebp esp
	// esp不能压栈保存
	assemble_buffer << "\n    push    eax"
					<< "\n    push    ebx"
					<< "\n    push    ecx"
					<< "\n    push    edx";
	//// TEST 输出ESP
	//assemble_buffer << "\n    mov     eax, esp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";

	assemble_buffer	<< "\n    push    ebp"
					<< "\n    mov     ebp,   esp"
					<< "\n    sub     esp,   " << 4 * (var_space + temp_space);
	//// TEST 输出EBP
	//assemble_buffer << "\n    mov     eax, ebp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";
	assemble_buffer << '\n';
	// 三. 在四元式表中找到主函数的开始地址，对四元式进行汇编
	for(vector<Quaternary>::const_iterator q_iter = Quaternary::GetFunctionBody(quaternarytable_.begin());
		Quaternary::END != q_iter->op_;
		++q_iter)
	{
		// 转换汇编码
		TranslateQuaternary(q_iter, 0, var_space, 0);
	}
	//assemble_buffer << "\n    push    offset  _String0"
	//				<< "\n    push    offset  _str_format"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp,   8";
	// 四. 主函数尾：还原工作
	assemble_buffer << '\n';
	//// TEST 输出EBP
	//assemble_buffer << "\n    mov     eax, ebp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";
	//assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// 还原栈顶指针
	assemble_buffer << "\n    mov     esp, ebp"	// 还原栈顶指针
					<< "\n    pop     ebp";
	//// TEST 输出ESP
	//assemble_buffer << "\n    mov     eax, esp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";

	assemble_buffer << "\n    pop     edx"
					<< "\n    pop     ecx"
					<< "\n    pop     ebx"
					<< "\n    pop     eax";
	//assemble_buffer << "\n    pop     0";
	assemble_buffer << "\n    call    ExitProcess";	// 这个有啥用？
	assemble_buffer << "\n_main  endp\n" << endl;
}

// 普通函数的汇编过程
// c_iter是该函数名在符号表中的表项的迭代器
void AssemblyMaker::OtherFunction(TokenTable::const_iterator c_iter) throw()
{
	// 一. 找到函数在四元式的BEGIN语句
	vector<Quaternary>::const_iterator q_iter = GetProcFuncIterInQuaternaryTable(c_iter);
	// 二. 得到参数所占的空间（单位：4bytes）
	int para_space = c_iter->value_;
	// 三. 得到局部变量的空间（单位：4bytes）
	int var_space = tokentable_.GetVariableSpace(c_iter + 1);
	// 四. 得到临时变量的空间（单位：4bytes）
	int temp_space = q_iter->src2_;
	// 五. 输出函数头，构造运行栈（为局部变量和临时变量分配空间）
	PrintQuaternaryComment(quaternarytable_, tokentable_, q_iter, assemble_buffer);
	assemble_buffer << "\n_" << c_iter->name_ << distance(tokentable_.begin(), c_iter) <<"  proc near";
	assemble_buffer << "\n    push    ebp"
					<< "\n    mov     ebp,   esp"
					<< "\n    sub     esp,   " << 4 * (var_space + temp_space);
	assemble_buffer << '\n';
	// 六. 找到函数主体在四元式中的语句
	//     然后逐个四元式生成汇编码
	for(q_iter = Quaternary::GetFunctionBody(q_iter);
		Quaternary::END != q_iter->op_;
		++q_iter)
	{
		TranslateQuaternary(q_iter, para_space, var_space, c_iter->level_);	
	}
	// 七. 输出函数尾
	// 函数返回的标号，使得在函数体中，任意地方返回时，只要跳转到这个标号就可以了
	PrintQuaternaryComment(quaternarytable_, tokentable_, q_iter, assemble_buffer);
	assemble_buffer << '\n' << c_iter->name_ << distance(tokentable_.begin(), c_iter) << "_Exit:";
	// 函数返回语句
	//assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// 还原栈顶指针(这样也可以，但如果中间栈会变动，就不好了。比如将函数返回值压栈存储时，可能会压入多个值，导致add的数量不确定。)
	assemble_buffer << "\n    mov     esp, ebp"										// 还原栈顶指针
					<< "\n    pop     ebp"
					<< "\n    ret";
	//assemble_buffer ;
//	assemble_buffer.width(8);
//	assemble_buffer.setf(std::ios::left);
	//assemble_buffer << "\n_" << std::setiosflags(ios::left)<< std::setw(8) << c_iter->name_ << "  endp\n";
	assemble_buffer << "\n_" << c_iter->name_ << distance(tokentable_.begin(), c_iter) << "  endp\n";
}

void AssemblyMaker::EndStatement() throw()
{
	assemble_buffer << "\nend start" << endl;
}

// 通过指向符号表的过程/函数的迭代器，找到其在四元式表中对应的BEGIN语句的迭代器
vector<Quaternary>::const_iterator AssemblyMaker::GetProcFuncIterInQuaternaryTable(TokenTable::const_iterator c_iter) const throw()
{
	for(vector<Quaternary>::const_iterator iter = quaternarytable_.begin();
		iter != quaternarytable_.end(); ++iter)
	{
		// 四元式的BEGIN语句中，dst_的值为过程/函数在符号表中的下标
		if(Quaternary::BEGIN == iter->op_
			&& iter->dst_ == distance(tokentable_.begin(), c_iter))
		{
			return iter;
		}
	}
	return quaternarytable_.end();
}

// 翻译四元式
void AssemblyMaker::TranslateQuaternary(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	PrintQuaternaryComment(quaternarytable_, tokentable_, c_iter, assemble_buffer);
	// 将四元式翻译为汇编码
	switch(c_iter->op_)
	{
	case Quaternary::NEG:
		TranslateNeg(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ADD:
		TranslateAdd(c_iter, para_num, var_space, level);
		break;
	case Quaternary::SUB:
		TranslateSub(c_iter, para_num, var_space, level);
		break;
	case Quaternary::MUL:
		TranslateMul(c_iter, para_num, var_space, level);
		break;
	case Quaternary::DIV:
		TranslateDiv(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ASG:
		TranslateAssign(c_iter, para_num, var_space, level);
		break;
	case Quaternary::AASG:
		TranslateArrayAssign(c_iter, para_num, var_space, level);
		break;
	
	case Quaternary::JMP:
		TranslateJmp(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JE:
		TranslateJe(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JNE:
		TranslateJne(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JG:
		TranslateJg(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JNG:
		TranslateJng(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JL:
		TranslateJl(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JNL:
		TranslateJnl(c_iter, para_num, var_space, level);
		break;
	case Quaternary::SETP:
		TranslateSetP(c_iter, para_num, var_space, level);
		break;
	case Quaternary::PROC_CALL:
	case Quaternary::FUNC_CALL:
		TranslateCall(c_iter, para_num, var_space, level);
		break;
	case Quaternary::RET:
		TranslateRet(c_iter, para_num, var_space, level);
		break;
	case Quaternary::STORE:
		TranslateStore(c_iter, para_num, var_space, level);
		break;
	case Quaternary::READ:
		TranslateRead(c_iter, para_num, var_space, level);
		break;
	case Quaternary::WRITE:
		TranslateWrite(c_iter, para_num, var_space, level);
		break;
	case Quaternary::LABEL:
		TranslateLabel(c_iter, para_num, var_space, level);
		break;
	// 以下三种情况不出现
	case Quaternary::BEGIN:
	case Quaternary::END:
	default:
		assert(false);
		break;
	}
}

// dst = -src1[offset]
// 源操作数可能是普通变量、数组变量或临时变量
// 注意，如果源操作数是立即数，则不需要装载至EAX再取反，而是直接在该函数体中取反，再存储至内存，这样可以只用一条汇编指令
// 然而，聪明的我已经在中间代码生成时，优化掉了立即数的取反，所以这里的源操作数不可能是立即数
void AssemblyMaker::TranslateNeg(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assert(Quaternary::IMMEDIATE_ADDRESSING != c_iter->method1_);	// 不信我们试试看？
	// 当源操作数与目的操作数相等时，可直接对内存进行操作
	if(c_iter->method1_ == c_iter->method3_ && c_iter->src1_ == c_iter->dst_)
	{
		// 此时两个操作数均不可能是数组，故array_offset参数给0
		OpGeneral(NEG, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
	else
	{
		// 装载源操作数至EAX
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
		// 取反
		assemble_buffer << "\n    neg     eax";
		// 将EAX存回内存
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
}

// Add的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
// 且不可能两个同时是立即数（已在expression的中间代码生成中优化）
// 目的操作数只可能是普通变量或临时变量
// TODO 优化：某个源操作数与目的操作数相等，且另一个源操作数是立即数时，可直接对内存做加法运算
void AssemblyMaker::TranslateAdd(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 第一个源操作数与目的操作数相等，且第二个源操作数是立即数时，直接对内存做加法运算
	if(c_iter->method1_ == c_iter->method3_ && c_iter->src1_ == c_iter->dst_ && Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		OpGeneralImmediate(ADD, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src2_);
	}
	// 第二个源操作数与目的操作数相等，且第一个源操作数是立即数时，直接对内存做加法运算
	else if(c_iter->method2_ == c_iter->method3_ && c_iter->src2_ == c_iter->dst_ && Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		OpGeneralImmediate(ADD, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src1_);
	}
	// 其余情况，当第一个操作数是立即数时
	else 
	{
		 if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
		{
			// 加载第二个操作数到EAX
			OpRegisterGeneral(MOV, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
			// 加上第一个立即数
			OpRegisterGeneral(ADD, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		}
		else // 当第一个操作数不是立即数时
		{
			// 加载第一个操作数到EAX
			OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
			// 加上第二个操作数
			OpRegisterGeneral(ADD, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		}
		// 将EAX中的结果保存到目的操作数
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
}

// 减法
// 两个源操作数都不可能是数组（指令格式不支持）
// 也都不可能是常数（中间代码生成时在Expression项被优化）
// TODO 优化：被减数与目的操作数相等，且减数是立即数时，可直接对内存做减法运算
void AssemblyMaker::TranslateSub(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 第一个源操作数与目的操作数相等，且第二个源操作数是立即数时，直接对内存做减法运算
	if(c_iter->method1_ == c_iter->method3_ && c_iter->src1_ == c_iter->dst_ && Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		OpGeneralImmediate(SUB, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src2_);
	}
	else
	{
		// 装载被减数到EAX
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// 减去第二个数
		OpRegisterGeneral(SUB, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// 存储结果
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
}

// Mul的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
// 且不可能两个同时是立即数（已在term的中间代码生成中优化）
// 目的操作数只可能是普通变量或临时变量
// 这里还可进一步优化。因为mul指令可以对内存进行操作，故实际上只需要加载一个操作数到EAX，用mul指令对另一个操作数的内存空间直接进行乘法
// 但那样实现起来比较麻烦。所以这里就统一加载到EAX和EDX中，再去做乘法
void AssemblyMaker::TranslateMul(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)	// 第一个操作数是立即数的情况
	{
		// 加载第一个操作数到EAX（因为立即数不能作IMUL的操作数）
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// 乘上第二个操作数
		OpGeneral(IMUL, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
	else
	{
		// 加载第二个操作数到EAX
		OpRegisterGeneral(MOV, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// 乘上第一个操作数
		OpGeneral(IMUL, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}

	// 将EAX中的结果保存到目的操作数
	OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
}

// Div的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
// 且不可能两个同时是立即数（已在term的中间代码生成中优化）
// 目的操作数只可能是普通变量或临时变量
// 这里比较重要的是，被除数的符号扩展。因为被除数是用EDX和EAX分别表示的高32位和低32位，故要用符号扩展设置EDX
// 同时要改成有符号的除法，这样才不会在输入负数时溢出
// 但乘法中的有无符号的乘法（mul, imul）似乎都差不多。影响的应该是结果中的EDX（64位结果中的高32位）
void AssemblyMaker::TranslateDiv(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 加载第一个操作数到EAX
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	// 第二个操作数，如果是立即数，要加载到寄存器中才可以除
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		OpRegisterGeneral(MOV, EBX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		assemble_buffer << "\n    CDQ";				// EDX相对EAX的符号位扩展
		assemble_buffer << "\n    idiv     ebx";	// 除寄存器
	}
	else	// 不是立即数，则可直接除
	{
		assemble_buffer << "\n    CDQ";				// EDX相对EAX的符号位扩展
		OpGeneral(IDIV, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
	// 将EAX中的结果保存到目的操作数
	OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
}

// 赋值语句
// 源操作数可以是立即数、普通变量、临时变量和数组元素
// 目的操作数可以是普通变量和临时变量
void AssemblyMaker::TranslateAssign(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 源操作数是立即数时，可直接存储至内存
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		OpGeneralImmediate(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src1_);
	}
	else
	{
		// 将源操作数装载到EAX中
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
		// 将EAX存储到目的操作数中
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);	// stupid bug fixed by mxf at 20:17 2/2 2016 (0 for parameter level)
	}
}


// 数组赋值语句
// 源操作数可以是立即数、普通变量、临时变量
// 目的操作数为数组
void AssemblyMaker::TranslateArrayAssign(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 源操作数是立即数时，可直接存储至内存
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		OpArrayImmediate(MOV, c_iter->dst_, c_iter->method2_, c_iter->offset2_, level, c_iter->src1_);
	}
	else
	{
		// 加载变量到EAX
		// 这里的操作数不可能是数组，故数组下标可为任意数（这里给零）
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// 将EAX的数据存储在数组里
		OpArrayRegister(MOV, c_iter->dst_, c_iter->method2_, c_iter->offset2_, level);
	}
}

// 无条件跳转
void AssemblyMaker::TranslateJmp(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer << "\n    jmp     " << GenerateLabelString(c_iter->dst_);
}
// 左右操作数相等时跳转
void AssemblyMaker::TranslateJe(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    je      " << GenerateLabelString(c_iter->dst_);
}
// 左右操作数不等时跳转
void AssemblyMaker::TranslateJne(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jne     " << GenerateLabelString(c_iter->dst_);
}
// 左>右则跳转
void AssemblyMaker::TranslateJg(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jg      " << GenerateLabelString(c_iter->dst_);
}
// 左<=右则跳转
void AssemblyMaker::TranslateJng(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jng     " << GenerateLabelString(c_iter->dst_);
}
// 左<右则跳转
void AssemblyMaker::TranslateJl(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jl      " << GenerateLabelString(c_iter->dst_);
}
// 左<=右则跳转
void AssemblyMaker::TranslateJnl(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jnl     " << GenerateLabelString(c_iter->dst_);
}

void AssemblyMaker::TranslateLabel(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer  << "\n" << GenerateLabelString(c_iter->dst_) << ":";
}

// 将参数压栈
// 参数可能是立即数、普通变量、数组、临时变量
void AssemblyMaker::TranslateSetP(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpGeneral(PUSH, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
}

// 这里的难度主要在于display区的控制
// 新函数的subfunc_level的取值为[1, level + 1]
// 当subfunc_level <= level时，复制当前函数的display区的前subfunc_level个表项
// 当subfunc_level == level + 1时，将当前函数的display区（共level项）复制给子函数后，再将EBP作为子函数新的一个display表项
void AssemblyMaker::TranslateCall(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 一. 获得被调用函数的level
	int subfunc_level = tokentable_.at(c_iter->dst_).level_;
	// 二. 根据调用者和被调用者的level复制display区
	int offset = 0;
	if(subfunc_level <= level)
	{
		for(int i = 0; i < subfunc_level; ++i)
		{
			offset = 4 * (1 + level - i);
			assemble_buffer << "\n    mov     eax, SS:[ebp + " << offset << "]"
							<< "\n    mov     SS:[esp - " << 4 * (i + 1) <<"], eax";
		}
	}
	else if(subfunc_level == level + 1)
	{
		for(int i = 0; i < level; ++i)
		{
			offset = 4 * (1 + level - i);
			assemble_buffer << "\n    mov     eax, SS:[ebp + " << offset << "]"
							<< "\n    mov     SS:[esp - " << 4 * (i + 1) <<"], eax";
		}
		assemble_buffer << "\n    mov     SS:[esp - " << 4 * subfunc_level <<"], ebp";
	}
	// 三. 调整栈顶指针
	assemble_buffer << "\n    sub     esp, " << 4 * subfunc_level;
	// 三. call调用
	assemble_buffer << "\n    call    _" << tokentable_.at(c_iter->dst_).name_ << c_iter->dst_;
	// 四. 恢复栈顶指针
	int subfunc_para_num = tokentable_.at(c_iter->dst_).value_;
	assemble_buffer  << "\n    add     esp, " << 4 * (subfunc_level + subfunc_para_num);
}

// 将目的操作数装入EAX，并进行函数返回
void AssemblyMaker::TranslateRet(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
	//LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX);
	// 找到函数的返回语句前的标号
	string exit_label = FindExitLabel(c_iter);
	assemble_buffer  << "\n    jmp     " << exit_label;
}


// 将EAX的数据储存起来
// 用在函数调用返回后，取得函数的返回值
// 储存的目的一般是临时变量，在优化过后也可能是普通变量，但不可能是数组（未予以数组优化）或立即数（非法）
void AssemblyMaker::TranslateStore(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
}

// 输入到dst[offset]
// 注：文法还没有支持到数组，但可以在这里先支持，最后再扩展文法
// 这里支持：普通变量和数组变量【临时变量不支持，因为不可能往临时变量中读数】
void AssemblyMaker::TranslateRead(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 将要读取的变量的地址装入EAX
	//LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX, true);
	OpRegisterGeneral(LEA, EAX, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
	// 将地址压栈
	assemble_buffer << "\n    push    eax";
	// 将输入格式字符串压栈
	if(TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)
	{
		assemble_buffer << "\n    push    offset  _char_format_s";
	}
	else
	{
		assemble_buffer << "\n    push    offset  _integer_format_s";	
	}
	assemble_buffer << "\n    call    scanf"
					<< "\n    add     esp, 8";
}

// 输出dst[offset]
void AssemblyMaker::TranslateWrite(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 将操作数压栈
	//PushGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level);
	OpGeneral(PUSH, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);	// 两者都可
	// 将格式字符串压栈
	if(Quaternary::STRING_ADDRESSING == c_iter->method3_)
	{
		assemble_buffer << "\n    push    offset  _string_format";
	}
	else if(TokenTableItem::CHAR == c_iter->dst_decoratetype_)
	{
		assemble_buffer << "\n    push    offset  _char_format_p";
	}
	else
	{
		assemble_buffer << "\n    push    offset  _integer_format_p";
	}
	// 调用printf函数
	assemble_buffer << "\n    call    printf"
					<< "\n    add     esp, 8";
}

// 单操作数的运算
void AssemblyMaker::OpGeneral(enum SINGLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		OpImmediate(op, index_or_value);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpVar(op, index_or_value, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpArray(op, index_or_value, array_addr_method, array_offset, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpTemp(op, index_or_value, var_space);
	}
	else if(Quaternary::STRING_ADDRESSING == addressingmethod)
	{
		assemble_buffer << "\n    " << SingleOperatorName[op] << "    offset  _String" << index_or_value;
	}
	else
	{
		assert(false);
	}
}

// 对常数的操作
void AssemblyMaker::OpImmediate(enum SINGLEOPERATOR op, int value) throw()
{
	assemble_buffer << "\n    " << SingleOperatorName[op] << "    " << value;
}

// 对普通变量的操作
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpVar(enum SINGLEOPERATOR op, int tokentable_index, int para_num, int level) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 0;
		if(n < para_num)	// 说明是参数
		{
			offset = 4 * (1 + level + para_num - n);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp + " << offset << "]";
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp - " << offset << "]";
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, dword ptr SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		if(n < extern_para_num)	// 说明是外层参数
		{
			offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebx + " << offset << "]";
		}
		else				    // 说明是外层局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebx - " << offset << "]";
		}
	}
}

// 对数组元素的操作
// 注意：数组不可能是函数的参数，故比LoadArray要少一个判断
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	这里的n即为外层函数的层次
void AssemblyMaker::OpArray(enum SINGLEOPERATOR op, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	int offset = 0;
	if(level == func_level)	// 判断是否为局部变量
	{
		// 数组的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp - " << offset << "]";
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebp），放在ECX中。此时数组元素的绝对偏移为ebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2"; // 乘4
			// 3. 计算出绝对偏移，放在EAX中（-(eax-ebp)）
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ecx]";
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的EBP的值，加载到EBX中
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, dword ptr SS:[ebp + " << offset << "]";
		// 二. 数组元素的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebx - " << offset << "]";
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebx），放在ECX中。此时数组元素的绝对偏移为ebx-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2"; // 乘4
			// 3. 计算出绝对偏移，放在EAX中（-(eax-ebx)）
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ecx]";
		}
	}
}

// 对临时变量的操作
// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpTemp(enum SINGLEOPERATOR op, int index, int var_space) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp - " << offset << "]";
}


// 将寄存器EAX的数据存储到内存中
// 可能的内存类型为：普通变量、数组变量、临时变量
// 根据取址方式的不同，调用不同的存储函数
// 第二个参数可能表示符号表中的index，也可能表示临时变量的index，故只命名为index
void AssemblyMaker::OpGeneralRegister(enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, enum REGISTER reg) throw()
{
	if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpVarRegister(op, index, para_num, level, reg);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpArrayRegister(op, index, array_addr_method, array_offset, level, reg);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpTempRegister(op, index, var_space, reg);
	}
	else
	{
		assert(false);
	}
}

// 将立即数存储到内存中
// 可能的内存类型为：普通变量、数组变量、临时变量
// 根据取址方式的不同，调用不同的存储函数
// 第二个参数可能表示符号表中的index，也可能表示临时变量的index，故只命名为index
void AssemblyMaker::OpGeneralImmediate(enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, int immediate_value) throw()
{
	if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpVarImmediate(op, index, para_num, level, immediate_value);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpArrayImmediate(op, index, array_addr_method, array_offset, level, immediate_value);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpTempImmediate(op, index, var_space, immediate_value);
	}
	else
	{
		assert(false);
	}
}

// 将数据从寄存器EAX存储到变量中
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpVarRegister(enum DOUBLEOPERATOR op, int tokentable_index, int para_num, int level, enum REGISTER reg) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断变量是否为函数的局部变量
	{
		int offset = 0;
		if(n < para_num)	// 说明是参数
		{
			offset = 4 * (1 + level + para_num - n);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp + " << offset << "], " << RegisterName[reg];
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp - " << offset << "], " << RegisterName[reg];
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		if(n < extern_para_num)	// 说明是外层参数
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebx + " << offset << "], " << RegisterName[reg];
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebx - " << offset << "], " << RegisterName[reg];
		}
	}
}

// 将立即数存储到变量中
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpVarImmediate(enum DOUBLEOPERATOR op, int tokentable_index, int para_num, int level, int immediate_value) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断变量是否为函数的局部变量
	{
		int offset = 0;
		if(n < para_num)	// 说明是参数
		{
			offset = 4 * (1 + level + para_num - n);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp + " << offset << "], " << immediate_value;
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp - " << offset << "], " << immediate_value;
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		if(n < extern_para_num)	// 说明是外层参数
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebx + " << offset << "], " << immediate_value;
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebx - " << offset << "], " << immediate_value;
		}
	}
}

// 注意：array不可能是参数
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpArrayRegister(enum DOUBLEOPERATOR op, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level, enum REGISTER reg) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	int offset = 0;
	if(level == func_level)	// 判断是否为局部变量
	{
		// 数组的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp - " << offset << "], " << RegisterName[reg];
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebp），放在ECX中。此时数组元素的绝对偏移为ebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// 乘4
			// 3. 计算出绝对偏移，放在EBX中（-(eax-ebp)）
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    dword ptr SS:[ecx], " << RegisterName[reg];
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的EBP的值，加载到EBX中
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 二. 数组元素的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebx - " << offset << "], " << RegisterName[reg];
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebx），放在EAX中。此时数组元素的绝对偏移为ebx-eax
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// 乘4
			// 3. 计算出绝对偏移，放在EAX中（-(eax-ebx)）
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    SS:[ecx], " << RegisterName[reg];
		}
	}
}

// 注意：array不可能是参数
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpArrayImmediate(enum DOUBLEOPERATOR op, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level, int immediate_value) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	int offset = 0;
	if(level == func_level)	// 判断是否为局部变量
	{
		// 数组的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp - " << offset << "], " << immediate_value;
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebp），放在ECX中。此时数组元素的绝对偏移为ebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// 乘4
			// 3. 计算出绝对偏移，放在EBX中（-(eax-ebp)）
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    dword ptr dword ptr SS:[ecx], " << immediate_value;
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的EBP的值，加载到EBX中
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 二. 数组元素的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebx - " << offset << "], " << immediate_value;
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebx），放在EAX中。此时数组元素的绝对偏移为ebx-eax
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// 乘4
			// 3. 计算出绝对偏移，放在EAX中（-(eax-ebx)）
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    dword ptr SS:[ecx], " << immediate_value;
		}
	}
}

// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpTempRegister(enum DOUBLEOPERATOR op, int index, int var_space, enum REGISTER reg) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp - " << offset << "], " << RegisterName[reg];
}

// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpTempImmediate(enum DOUBLEOPERATOR op, int index, int var_space, int immediate_value) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp - " << offset << "], " << immediate_value;
}

// 双操作数的运算（第一个操作数是寄存器，第二个操作数是各种类型的内存变量或立即数）
void AssemblyMaker::OpRegisterGeneral(enum DOUBLEOPERATOR op, enum REGISTER reg, Quaternary::AddressingMethod addressingmethod, int index_or_value, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		OpRegisterImmediate(op, reg, index_or_value);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpRegisterVar(op, reg, index_or_value, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpRegisterArray(op, reg, index_or_value, array_addr_method, array_offset, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpRegisterTemp(op, reg, index_or_value, var_space);
	}
	else if(Quaternary::STRING_ADDRESSING == addressingmethod)
	{
		assemble_buffer << "\n    " << DoubleOperatorName[op] << "    offset  _String" << index_or_value;
	}
	else
	{
		assert(false);
	}
}

// 对常数的操作
void AssemblyMaker::OpRegisterImmediate(enum DOUBLEOPERATOR op, enum REGISTER reg, int value) throw()
{
	//TODO 通过输出格式控制来优化效率
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", "<< value;
}

// 对普通变量的操作
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpRegisterVar(enum DOUBLEOPERATOR op, enum REGISTER reg, int tokentable_index, int para_num, int level) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 0;
		if(n < para_num)	// 说明是参数
		{
			offset = 4 * (1 + level + para_num - n);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp + " << offset << "]";
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		if(n < extern_para_num)	// 说明是外层参数
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx + " << offset << "]";
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
	}
}

// 对数组元素的操作
// 注意：数组不可能是函数的参数，故比LoadArray要少一个判断
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	这里的n即为外层函数的层次
void AssemblyMaker::OpRegisterArray(enum DOUBLEOPERATOR op, enum REGISTER reg, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	int offset = 0;
	if(level == func_level)	// 判断是否为局部变量
	{
		// 数组的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebp），放在ECX中。此时数组元素的绝对偏移为ebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// 乘4
			// 3. 计算出绝对偏移，放在EBX中（-(eax-ebp)）
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", dword ptr SS:[ecx]";
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的EBP的值，加载到EBX中
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 二. 数组元素的偏移量的计算
		// 如果数组下标是立即数类型，则偏移量就是array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
		else	// 否则，就要取得数组下标（某个变量）的值，在汇编程序中计算其偏移量
		{
			// 1. 先把数组下标的变量的值放入ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. 计算出实际的数组元素的相对偏移（相对于ebx），放在EAX中。此时数组元素的绝对偏移为ebx-eax
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// 乘4
			// 3. 计算出绝对偏移，放在EAX中（-(eax-ebx)）
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. 执行该执行的操作
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", dword ptr SS:[ecx]";
		}
	}
}
//{
//	// 确定变量的层次
//	int var_level = tokentable_.at(tokentable_index).level_;
//	// 确定函数的层次（比变量层次少1）
//	int func_level = var_level - 1;
//	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
//	if(level == func_level)	// 判断是否为局部变量
//	{
//		int offset = 4 * (n + 1 + array_offset);
//		assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
//	}
//	else	// 外层次的变量
//	{
//		// 一. 获得外层次的EBP的值，加载到EBX中
//		int offset = 4 * (1 + level - func_level);
//		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
//		// 二. 加载外层的变量
//		offset = 4 * (n + 1 + array_offset);
//		assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
//	}
//}

// 对临时变量的操作
// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpRegisterTemp(enum DOUBLEOPERATOR op, enum REGISTER reg, int index, int var_space) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
}


// 根据四元式表中某项的迭代器，找到它所在的函数的返回语句块前的标号
// 这里有两种找法，一种向上查找到BEGIN语句，获得函数对应的符号表位置，这要求END语句中包含函数对应的符号表位置
// 另一种向下查找到END语句，这就要求END语句中包含函数对应的符号表位置
// 两种方法的要求都符合，然而第二种更简单，故采用第二种
string AssemblyMaker::FindExitLabel(vector<Quaternary>::const_iterator c_iter) throw()
{
	// 一. 找到函数在四元式表中的END语句
	while(Quaternary::END != c_iter->op_)
	{
		++c_iter;
	}
	// 二. 找到函数在符号表中的下标
	int tokentable_index = c_iter->dst_;
	// 三. 找到函数的名字
	std::ostringstream buffer;
	// 四. 返回函数的返回语句块前的标号
	buffer << tokentable_.at(tokentable_index).name_ << tokentable_index << "_Exit";
	return buffer.str();
}

// 通过label标号生成label字符串
string AssemblyMaker::GenerateLabelString(int label_index)	
{
	std::ostringstream buffer;
	buffer << "_label" << label_index;
	return buffer.str();
}

// 寄存器名
const char * const AssemblyMaker::RegisterName[4] = {"eax", "ebx", "ecx", "edx"};
const char * const AssemblyMaker::SingleOperatorName[4] = {"neg", "push", "imul", "idiv"};
const char * const AssemblyMaker::DoubleOperatorName[4] = {"mov ", "add ", "sub ", "lea "};