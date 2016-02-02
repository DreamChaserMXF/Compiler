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
	assemble_buffer << "\nincludelib .\\masm32\\lib\\msvcrt.lib";
	assemble_buffer << "\nincludelib .\\masm32\\lib\\kernel32.lib";
	assemble_buffer << "\ninclude .\\masm32\\include\\msvcrt.inc";
	assemble_buffer << "\ninclude .\\masm32\\include\\kernel32.inc";
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
	assemble_buffer << "\n    _string_format       db '%s ', 0";
	for(size_t i = 0; i < stringtable_.size(); ++i)
	{
		assemble_buffer << "\n    _String" << i << "           db '" << stringtable_[i] << "', 0"; 
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
					<< "\n    push    edx";
	assemble_buffer	<< "\n    push    ebp"
					<< "\n    mov     ebp,   esp"
					<< "\n    sub     esp,   " << 4 * (var_space + temp_space);
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
	assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// 还原栈顶指针
					<< "\n    pop     ebp";
	assemble_buffer << "\n    pop     edx"
					<< "\n    pop     ebx"
					<< "\n    pop     eax";
	//assemble_buffer << "\n    pop     0";
	assemble_buffer << "\n    call    ExitProcess";
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
	assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// 还原栈顶指针
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
void AssemblyMaker::TranslateQuaternary(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
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
// 注意，如果源操作数是立即数，则不需要装载至EAX再取反，而是直接在编译过程中取反，再存储至内存，这样可以只用一条汇编指令
// 然而，聪明的我已经在中间代码生成时，优化掉了立即数的取反，所以这里的源操作数不可能是立即数
void AssemblyMaker::TranslateNeg(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assert(Quaternary::IMMEDIATE_ADDRESSING != c_iter->method1_);	// 不信我们试试看？
	// 装载源操作数至EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, c_iter->offset2_, para_num, var_space, level, EAX);
	// 取反
	assemble_buffer << "\n    neg     eax";
	// 将EAX存回内存
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// Add的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
// 且不可能两个同时是立即数（已在expression的中间代码生成中优化）
// 目的操作数只可能是普通变量或临时变量
void AssemblyMaker::TranslateAdd(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 当第一个操作数是立即数时
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		// 加载第二个操作数到EAX
		LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EAX);
		// 加上第一个立即数
		assemble_buffer << "\n    add     eax, " << c_iter->src1_;
	}
	else if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)	// 当第二个操作数是立即数时
	{
		// 加载第一个操作数到EAX
		LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
		// 加上第二个立即数
		assemble_buffer << "\n    add     eax, " << c_iter->src2_;
	}
	else	// 左右操作数都不是立即数
	{
		// 加载第一个操作数到EAX
		LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
		// 加载第二个操作数到EDX
		LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
		// 相加
		assemble_buffer << "\n    add     eax, edx";
	}
	// 将EAX中的结果保存到目的操作数
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// 减法
// 两个源操作数都不可能是数组（指令格式不支持）
// 也都不可能是常数（中间代码生成时在Expression项被优化）
void AssemblyMaker::TranslateSub(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 装载被减数到EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	// 根据减数是否为立即数，进行处理
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		// EAX 减去立即数
		assemble_buffer << "\n    sub     eax, " << c_iter->src2_;
	}
	else
	{
		// 装载第二个操作数到EDX
		LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
		// EAX 减去 EDX
		assemble_buffer << "\n    sub     eax, edx";
	}
	// 存储结果
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// Mul的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
// 且不可能两个同时是立即数（已在term的中间代码生成中优化）
// 目的操作数只可能是普通变量或临时变量
// 这里还可进一步优化。因为mul指令可以对内存进行操作，故实际上只需要加载一个操作数到EAX，用mul指令对另一个操作数的内存空间直接进行乘法
// 但那样实现起来比较麻烦。所以这里就统一加载到EAX和EDX中，再去做乘法
void AssemblyMaker::TranslateMul(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 加载第一个操作数到EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	// 加载第二个操作数到EDX
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	// 相乘
	assemble_buffer << "\n    mul     edx";
	// 将EAX中的结果保存到目的操作数
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// Div的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
// 且不可能两个同时是立即数（已在term的中间代码生成中优化）
// 目的操作数只可能是普通变量或临时变量
// 类似乘法，这里还可进一步优化。但由于实现过于复杂，故暂不优化。
// 这里比较重要的是，被除数的符号扩展。因为被除数是用EDX和EAX分别表示的高32位和低32位，故要用符号扩展设置EDX
// 同时要改成有符号的除法，这样才不会在输入负数时溢出
// 但乘法中的有无符号的乘法（mul, imul）似乎都差不多。
void AssemblyMaker::TranslateDiv(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 加载第一个操作数到EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	// 加载第二个操作数到EBX
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EBX);
	// Imp! 除法时，EDX是被除数的高位，要进行符号扩展	
	assemble_buffer << "\n    CDQ";
	// 相除
	assemble_buffer << "\n    idiv     ebx";	// Imp! idiv:有符号的除法，如果是div的话，操作数为负数时容易溢出报错
	// 将EAX中的结果保存到目的操作数
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// 赋值语句
// 源操作数可以是立即数、普通变量、临时变量和数组元素
// 目的操作数可以是普通变量和临时变量
void AssemblyMaker::TranslateAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 将源操作数装载到EAX中
	LoadGeneral(c_iter->method1_, c_iter->src1_, c_iter->offset2_, para_num, var_space, level, EAX);
	// 将EAX存储到目的操作数中
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);	// stupid bug fixed by mxf at 20:17 2/2 2016
}


// 数组赋值语句
// 源操作数可以是立即数、普通变量、临时变量
// 目的操作数为数组
void AssemblyMaker::TranslateArrayAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 加载立即数或变量到EAX
	// 这里的操作数不可能是数组，故第三个参数为任意数（这里给零）
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	//if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//{
	//	// 将立即数放在EAX中
	//	assemble_buffer << "\n    mov     eax, " << c_iter->src1_;
	//}
	//else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//{
	//	// 将普通变量加载到EAX中
	//	LoadVar(c_iter->src1_, para_num, level, EAX);
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//{
	//	// 将临时变量加载到EAX中
	//	LoadTemp(c_iter->src1_, var_space, EAX);
	//}
	//else
	//{
	//	assert(false);
	//}

	// 将EAX的数据存储在数组里
	StoreArray(c_iter->dst_, c_iter->src2_, para_num, level);
	
}

// 将EAX的数据储存起来
// 用在函数调用返回后，取得函数的返回值
// 储存的目的一般是临时变量，在优化过后也可能是普通变量，但不可能是数组或立即数
void AssemblyMaker::TranslateStore(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// 输入到dst[offset]
// 注：文法还没有支持到数组，但可以在这里先支持，最后再扩展文法
// 这里支持：普通变量和数组变量【临时变量不支持，因为不可能往临时变量中读数】
void AssemblyMaker::TranslateRead(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 将要读取的变量的地址装入EAX
	LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX, true);
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
void AssemblyMaker::TranslateWrite(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method3_)		// 写立即数
	{
		if(TokenTableItem::INTEGER == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _integer_format_p";
		}
		else if(TokenTableItem::CHAR == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _char_format_p";
		}
		else
		{
			assert(false);
		}
	}
	else if(Quaternary::STRING_ADDRESSING == c_iter->method3_)	// 写字符串
	{
		assemble_buffer << "\n    push    offset  _String" << c_iter->dst_
						<< "\n    push    offset  _string_format";
	}
	else	// 加载变量到EAX，将EAX压栈并加载格式字符串到栈顶
	{
		
		LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX);// 加载变量
		assemble_buffer << "\n    push    eax";	// EAX压栈
		// 加载格式字符串
		if((Quaternary::VARIABLE_ADDRESSING == c_iter->method3_
			|| Quaternary::ARRAY_ADDRESSING == c_iter->method3_)
			&& TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)	// 字符型的情况（操作数是变量或数组，且修饰类型为字符型）
		{
			assemble_buffer << "\n    push    offset  _char_format_p";
		}
		else	// 整型的情况（操作数为临时变量或非字符型的变量/数组）
		{
			assemble_buffer << "\n    push    offset  _integer_format_p";
		}
	}
	// 调用printf函数
	assemble_buffer << "\n    call    printf"
					<< "\n    add     esp, 8";
}

// 无条件跳转
void AssemblyMaker::TranslateJmp(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer << "\n    jmp     " << LabelStringFormat(c_iter->dst_);
}
// 左右操作数相等时跳转
void AssemblyMaker::TranslateJe(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    je      " << LabelStringFormat(c_iter->dst_);
}
// 左右操作数不等时跳转
void AssemblyMaker::TranslateJne(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jne     " << LabelStringFormat(c_iter->dst_);
}
// 左>右则跳转
void AssemblyMaker::TranslateJg(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jg      " << LabelStringFormat(c_iter->dst_);
}
// 左<=右则跳转
void AssemblyMaker::TranslateJng(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jng     " << LabelStringFormat(c_iter->dst_);
}
// 左<右则跳转
void AssemblyMaker::TranslateJl(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jl      " << LabelStringFormat(c_iter->dst_);
}
// 左<=右则跳转
void AssemblyMaker::TranslateJnl(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jnl     " << LabelStringFormat(c_iter->dst_);
}

// 将参数压栈
// 参数可能是立即数、普通变量、数组、临时变量
void AssemblyMaker::TranslateSetP(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	PushGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level);
}

// 这里的难度主要在于display区的控制
// 新函数的subfunc_level的取值为[1, level + 1]
// 当subfunc_level <= level时，复制当前函数的display区的前subfunc_level个表项
// 当subfunc_level == level + 1时，将当前函数的display区（共level项）复制给子函数后，再将EBP作为子函数新的一个display表项
void AssemblyMaker::TranslateCall(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
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
void AssemblyMaker::TranslateRet(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX);
	// 找到函数的返回语句前的标号
	string exit_label = FindExitLabel(c_iter);
	assemble_buffer  << "\n    jmp     " << exit_label;
}

void AssemblyMaker::TranslateLabel(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer  << "\n" << LabelStringFormat(c_iter->dst_) << ":";
}

// 将立即数/普通变量/数组变量/临时变量装载到寄存器reg
// 根据取址方式的不同，调用不同的装载函数
// 第二个参数可能是符号表index，可能是临时变量的index，也可能是立即数值，故命名为index_or_value
// 每次调用时，第一个参数，即取址方式，和最后一个参数，寄存器的值，都是必须有效的。且所有调用的意义都相同。
// 除此之外，立即数依赖的参数为：index_or_value
// 普通变量依赖的参数为：index_or_value, para_num, level
// 数组变量依赖的参数为：index_or_value, array_offset, var_space, level
// 临时变量依赖的参数为：index_or_value, var_space
// 严格地说，只要在调用时，将可能依赖的参数赋予有效的值即可
// 但在一般调用时，往往不确定变量类型的情况，所以就将参数全部填入有效值
// 当不可能是数组元素时，一般地，array_offset赋0即可（实际上无影响）
void AssemblyMaker::LoadGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level, enum REGISTER reg, bool load_addr) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		LoadImmediate(index_or_value, reg);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		LoadVar(index_or_value, para_num, level, reg, load_addr);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		LoadArray(index_or_value, array_offset, para_num, level, reg, load_addr);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		LoadTemp(index_or_value, var_space, reg);
	}
	else
	{
		assert(false);
	}
}

// 将常数value装载到寄存器reg
void AssemblyMaker::LoadImmediate(int value, enum REGISTER reg) throw()
{
	assemble_buffer << "\n    mov     " << RegisterName[reg] << ", " << value;
}

// 将变量或变量的地址装载到寄存器中
// tokentable_index为变量在符号表中的位置
// para_num表示当前函数的参数个数
// level表示当前函数的静态层次（即在符号表中的层次）
// reg代表装入哪个寄存器
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::LoadVar(int tokentable_index, int para_num, int level, enum REGISTER reg, bool load_addr) throw()
{
	// 确定操作的方式：装载变量还是装载变量的地址
	const char *action = load_addr ? "lea" : "mov";
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量在所属函数的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 0;
		if(n < para_num)	// 说明是参数
		{
			offset = 4 * (1 + level + para_num - n);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebp + " << offset << "]";
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    " << action << "     ebx, SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		if(n < extern_para_num)	// 说明是外层参数
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebx + " << offset << "]";
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
	}
}

// 装载数组元素或其地址到寄存器reg
// 注意：array不可能是函数的参数，故比LoadArray要少一个判断
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	这里的n即为外层函数的层次
void AssemblyMaker::LoadArray(int tokentable_index, int array_offset, int para_num, int level, enum REGISTER reg, bool load_addr) throw()
{
	// 确定操作的方式：装载变量还是装载变量的地址
	const char *action = load_addr ? "lea" : "mov";
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    " << action << "     ebx, SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
	}
}
// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::LoadTemp(int index, int var_space, enum REGISTER reg) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
}

// 将寄存器EAX的数据存储到内存中
// 可能的内存类型为：普通变量、数组变量、临时变量
// 根据取址方式的不同，调用不同的存储函数
// 第二个参数可能表示符号表中的index，也可能表示临时变量的index，故只命名为index
void AssemblyMaker::StoreGeneral(Quaternary::AddressingMethod addressingmethod, int index, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		StoreVar(index, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		StoreArray(index, array_offset, para_num, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		StoreTemp(index, var_space);
	}
	else
	{
		assert(false);
	}
}

// 将变量从寄存器EAX存储到内存
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::StoreVar(int tokentable_index, int para_num, int level) throw()
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
			assemble_buffer << "\n    mov     SS:[ebp + " << offset << "], eax";
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
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
			assemble_buffer << "\n    mov     SS:[ebx + " << offset << "], eax";
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    mov     SS:[ebx - " << offset << "], eax";
		}
	}
}

// 注意：array不可能是参数
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::StoreArray(int tokentable_index, int array_offset, int para_num, int level) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     SS:[ebx - " << offset << "], eax";
	}
}

// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::StoreTemp(int index, int var_space) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
}

// 将立即数/普通变量/数组变量/临时变量装载到寄存器reg
// 根据取址方式的不同，调用不同的装载函数
// 第二个参数可能是符号表index，可能是临时变量的index，也可能是立即数值，故命名为index_or_value
// 每次调用时，第一个参数，即取址方式，和最后一个参数，寄存器的值，都是必须有效的。且所有调用的意义都相同。
// 除此之外，立即数依赖的参数为：index_or_value
// 普通变量依赖的参数为：index_or_value, para_num, level
// 数组变量依赖的参数为：index_or_value, array_offset, var_space, level
// 临时变量依赖的参数为：index_or_value, var_space
// 严格地说，只要在调用时，将可能依赖的参数赋予有效的值即可
// 但在一般调用时，往往不确定变量类型的情况，所以就将参数全部填入有效值
// 当不可能是数组元素时，一般地，array_offset赋0即可（实际上无影响）
void AssemblyMaker::PushGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		PushImmediate(index_or_value);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		PushVar(index_or_value, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		PushArray(index_or_value, array_offset, para_num, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		PushTemp(index_or_value, var_space);
	}
	else
	{
		assert(false);
	}
}

// 将常数value装载到寄存器reg
void AssemblyMaker::PushImmediate(int value) throw()
{
	assemble_buffer << "\n    push    " << value;
}

// 将变量压栈
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::PushVar(int tokentable_index, int para_num, int level) throw()
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
			assemble_buffer << "\n    push    SS:[ebp + " << offset << "]";
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    push    SS:[ebp - " << offset << "]";
		}
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    push    SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		if(n < extern_para_num)	// 说明是外层参数
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    push    SS:[ebx + " << offset << "]";
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    push    SS:[ebx - " << offset << "]";
		}
	}
}

// 将数组元素压栈
// 注意：数组不可能是函数的参数，故比LoadArray要少一个判断
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	这里的n即为外层函数的层次
void AssemblyMaker::PushArray(int tokentable_index, int array_offset, int para_num, int level) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n   push    SS:[ebp - " << offset << "]";
	}
	else	// 外层次的变量
	{
		// 一. 获得外层次的参数个数
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// 二. 获得外层次的EBP的值，加载到EBX中
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    push    SS:[ebp + " << offset << "]";
		// 三. 加载外层的变量
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    push    SS:[ebx - " << offset << "]";
	}
}
// 将临时变量压栈
// 地址计算公式：
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::PushTemp(int index, int var_space) throw()
{
	// 这里的index就是上式中的n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    push    SS:[ebp - " << offset << "]";
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
string AssemblyMaker::LabelStringFormat(int label_index)	
{
	std::ostringstream buffer;
	buffer << "_label" << label_index;
	return buffer.str();
}

// 寄存器名
 const char * const AssemblyMaker::RegisterName[3] = 
 {"eax", "ebx", "edx"};