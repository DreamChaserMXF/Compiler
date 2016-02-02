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
	assemble_buffer << "\n    _integer_format    db '%d ', 0";
	assemble_buffer << "\n    _char_format       db '%c ', 0";
	assemble_buffer << "\n    _string_format     db '%s ', 0";
//	assemble_buffer << "\n    _String0		DB	'123', 0";
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
	//assemble_buffer << "\n_" << std::setiosflags(ios::right)<< std::setw(8) << c_iter->name_ << "  proc near";
	//assemble_buffer.width(8);
	//assemble_buffer.setf(std::ios::left);
	assemble_buffer << "\n_" << c_iter->name_ << "  proc near";
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
	assemble_buffer << '\n';
	assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// 还原栈顶指针
					<< "\n    pop     ebp"
					<< "\n    ret";
	//assemble_buffer ;
//	assemble_buffer.width(8);
//	assemble_buffer.setf(std::ios::left);
	//assemble_buffer << "\n_" << std::setiosflags(ios::left)<< std::setw(8) << c_iter->name_ << "  endp\n";
	assemble_buffer << "\n_" << c_iter->name_ << "  endp\n";
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

// TODO 翻译四元式
void AssemblyMaker::TranslateQuaternary(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 先以注释形式输出待翻译的四元式
	assemble_buffer << "\n    ;";
	// 输出序号
	assemble_buffer.width(3);
	assemble_buffer.setf(ios::right);
	assemble_buffer << distance(quaternarytable_.begin(), c_iter) << "  ";
	// 输出操作符
	assemble_buffer.width(9);
	assemble_buffer.setf(ios::right);
	assemble_buffer << Quaternary::OPCodeString[c_iter->op_] << " ";
	// 输出操作数
	PrintOperand(c_iter->method1_, c_iter->src1_, tokentable_, assemble_buffer);
	PrintOperand(c_iter->method2_, c_iter->src2_, tokentable_, assemble_buffer);
	PrintOperand(c_iter->method3_, c_iter->dst_, tokentable_, assemble_buffer);
	// 将四元式翻译为汇编码
	switch(c_iter->op_)
	{
	case Quaternary::NEG:
		//TranslateNeg(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ADD:
		TranslateAdd(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ASG:
		TranslateAssign(c_iter, para_num, var_space, level);
		break;
	case Quaternary::AASG:
		TranslateArrayAssign(c_iter, para_num, var_space, level);
		break;
	case Quaternary::WRITE:
		TranslateWrite(c_iter, para_num, var_space, level);
		break;
	case Quaternary::PROC_CALL:
	case Quaternary::FUNC_CALL:
		TranslateCall(c_iter, para_num, var_space, level);
		break;
	default:
		break;
	}
}

// Add的左右操作数只可能是：立即数、普通变量、临时变量，不会是数组
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

	//if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//{
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	//	{
	//		// 因为在四元式生成时有过对两个常数的运算的优化，所以不可能有两个立即数的情况
	//		assert(false);
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加载第二个普通变量到EAX
	//		LoadVar(c_iter->src2_, para_num, level, EAX);
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加载第二个临时变量到EAX
	//		LoadTemp(c_iter->src2_, var_space, EAX);
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//	// 加上第一个立即数
	//	assemble_buffer << "\n    add     eax, " << c_iter->src1_;
	//}
	//else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//{
	//	// 加载第一个普通变量到EAX
	//	LoadVar(c_iter->src1_, para_num, level, EAX);
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加上第二个立即数
	//		assemble_buffer << "\n    add     eax, " << c_iter->src2_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加载第二个普通变量到EDX
	//		LoadVar(c_iter->src2_, para_num, level, EDX);
	//		// EAX与EDX相加
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加载第二个临时变量到EDX
	//		LoadTemp(c_iter->src2_, var_space, EDX);
	//		// EAX与EDX相加
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//{
	//	// 加载第一个临时变量到EAX
	//	LoadTemp(c_iter->src1_, var_space, EAX);
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加上第二个立即数
	//		assemble_buffer << "\n    add     eax, " << c_iter->src2_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加载第二个普通变量到EDX
	//		LoadVar(c_iter->src2_, para_num, level, EDX);
	//		// EAX与EDX相加
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method2_)
	//	{
	//		// 加载第二个临时变量到EDX
	//		LoadTemp(c_iter->src2_, var_space, EDX);
	//		// EAX与EDX相加
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//}
	//else
	//{
	//	assert(false);
	//}
	//// 将EAX存储给目的操作数（普通变量或临时变量）
	//if(Quaternary::VARIABLE_ADDRESSING == c_iter->method3_)
	//{
	//	StoreVar(c_iter->dst_, para_num, level);
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method3_)
	//{
	//	StoreTemp(c_iter->dst_, var_space);
	//}
}

// 赋值语句
// 源操作数可以是立即数、普通变量、临时变量和数组元素
// 目的操作数可以是普通变量和临时变量
void AssemblyMaker::TranslateAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// 将源操作数装载到EAX中
	LoadGeneral(c_iter->method1_, c_iter->src1_, c_iter->offset2_, para_num, var_space, level, EAX);
	// 将EAX存储到目的操作数中
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, 0);
	//if(Quaternary::VARIABLE_ADDRESSING == c_iter->method3_)
	//{
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将立即数放在EAX中
	//		assemble_buffer << "\n    mov     eax, " << c_iter->src1_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将普通变量加载到EAX中
	//		LoadVar(c_iter->src1_, para_num, level, EAX);
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将临时变量加载到EAX中
	//		LoadTemp(c_iter->src1_, var_space, EAX);
	//	}
	//	else if(Quaternary::ARRAY_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将数组元素加载到EAX中
	//		LoadArray(c_iter->src1_, c_iter->src2_, para_num, level, EAX);
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//	// 将EAX的数据存储在普通变量里
	//	StoreVar(c_iter->dst_, para_num, level);
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method3_)
	//{
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将立即数放在EAX中
	//		assemble_buffer << "\n    mov     eax, " << c_iter->src1_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将普通变量加载到EAX中
	//		LoadVar(c_iter->src1_, para_num, level, EAX);
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将临时变量加载到EAX中
	//		LoadTemp(c_iter->src1_, var_space, EAX);
	//	}
	//	else if(Quaternary::ARRAY_ADDRESSING == c_iter->method1_)
	//	{
	//		// 将数组元素加载到EAX中
	//		LoadArray(c_iter->src1_, c_iter->src2_, para_num, level, EAX);
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//	// 将EAX的数据存储在临时变量里
	//	StoreTemp(c_iter->dst_, var_space);
	//}
	//else
	//{
	//	assert(false);
	//}
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


void AssemblyMaker::TranslateWrite(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method3_)		// 写立即数
	{
		if(TokenTableItem::INTEGER == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _integer_format";
		}
		else if(TokenTableItem::CHAR == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _char_format";
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
			assemble_buffer << "\n    push    offset  _char_format";
		}
		else	// 整型的情况（操作数为临时变量或非字符型的变量/数组）
		{
			assemble_buffer << "\n    push    offset  _integer_format";
		}
	}
	//else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method3_)	// 写普通变量或数组变量
	//{
	//	LoadVar(c_iter->dst_, para_num, level, EAX);
	//	assemble_buffer << "\n    push    eax";
	//	if(TokenTableItem::INTEGER == tokentable_.at(c_iter->dst_).decoratetype_)	// 整型变量
	//	{
	//		assemble_buffer << "\n    push    offset  _integer_format";
	//	}
	//	else if(TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)	// 字符型变量
	//	{			
	//		assemble_buffer << "\n    push    offset  _char_format";
	//	}
	//}
	//else if(Quaternary::ARRAY_ADDRESSING == c_iter->method3_)	// 写数组变量
	//{
	//	LoadArray(c_iter->dst_, c_iter->offset2_, para_num, level, EAX);
	//	assemble_buffer << "\n    push    eax";
	//	if(TokenTableItem::INTEGER == tokentable_.at(c_iter->dst_).decoratetype_)	// 整型变量
	//	{
	//		assemble_buffer << "\n    push    offset  _integer_format";
	//	}
	//	else if(TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)	// 字符型变量
	//	{			
	//		assemble_buffer << "\n    push    offset  _char_format";
	//	}
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method3_)	// 写临时变量
	//{
	//	LoadTemp(c_iter->dst_, var_space, EAX);
	//	assemble_buffer << "\n    push    eax"
	//					<< "\n    push    offset  _integer_format";
	//}
	//else
	//{
	//	assert(false);
	//}
	// 调用printf函数
	assemble_buffer << "\n    call    printf"
					<< "\n    add     esp, 8";
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
	assemble_buffer << "\n    call    _" << tokentable_.at(c_iter->dst_).name_;
	// 四. 恢复栈顶指针
	int subfunc_para_num = tokentable_.GetParameterNum(c_iter->dst_);
	assemble_buffer  << "\n    add     esp, " << 4 * (subfunc_level + subfunc_para_num);
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
void AssemblyMaker::LoadGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level, enum REGISTER reg) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		LoadImmediate(index_or_value, reg);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		LoadVar(index_or_value, para_num, level, reg);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		LoadArray(index_or_value, array_offset, para_num, level, reg);
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

// 将变量装载到寄存器中
// tokentable_index为变量在符号表中的位置
// para_num表示当前函数的参数个数
// level表示当前函数的静态层次（即在符号表中的层次）
// reg代表装入哪个寄存器
// 地址计算公式：
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::LoadVar(int tokentable_index, int para_num, int level, enum REGISTER reg) throw()
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
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
		}
		assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
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
			assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebx + " << offset << "]";
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
	}
}

// 注意：array不可能是参数
// 地址计算公式：
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	这里的n即为外层函数的层次
void AssemblyMaker::LoadArray(int tokentable_index, int array_offset, int para_num, int level, enum REGISTER reg) throw()
{
	// 确定变量的层次
	int var_level = tokentable_.at(tokentable_index).level_;
	// 确定函数的层次（比变量层次少1）
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// 计算该变量的相对地址
	if(level == func_level)	// 判断是否为局部变量
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
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
		assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
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
		}
		else				// 说明是局部变量
		{
			offset = 4 * (n + 1);
		}
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
		if(n < extern_para_num)	// 说明是外层参数
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
		}
		else				    // 说明是外层局部变量
		{
			int offset = 4 * (n + 1);
		}
		assemble_buffer << "\n    mov     SS:[ebx - " << offset << "], eax";
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
// 寄存器名
 const char * const AssemblyMaker::RegisterName[2] = 
 {"eax", "edx"};