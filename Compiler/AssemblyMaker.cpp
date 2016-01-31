#include "AssemblyMaker.h"
#include <iostream>
#include <fstream>
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
	// TODO traverse the tokentable
	for(TokenTable::const_iterator c_iter = tokentable_.begin();
		c_iter != tokentable_.end(); ++c_iter)
	{
		if(TokenTableItem::PROCEDURE == c_iter->itemtype_)
		{
			OtherProcedure(GetVariableSpace(c_iter + 1), GetProcFuncIterInQuaternaryTable(c_iter));
		}
		else if(TokenTableItem::FUNCTION == c_iter->itemtype_)
		{
			OtherFunction(GetVariableSpace(c_iter + 1), GetProcFuncIterInQuaternaryTable(c_iter));
		}
	}
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
	assemble_buffer << "\nincludelib .\\masm32\\include\\msvcrt.inc";
	assemble_buffer << "\nincludelib .\\masm32\\include\\kernel32.inc";
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
	assemble_buffer << "\n    _str_format        db '%s ', 0";
	assemble_buffer << endl;
}
void AssemblyMaker::CodeBeginSegment() throw()
{
	assemble_buffer << "\n.CODE" << endl;
}
void AssemblyMaker::MainFunction() throw()
{
	// 主函数声明
	assemble_buffer << "\n.start:\n";				// 开始位置
	assemble_buffer << "\nmain:  proc far" << endl;	// 跳转方式
	// 主函数头
	// 一. 找出主函数中变量的存储空间
	int var_space = GetVariableSpace(tokentable_.begin());;

	//TokenTable::const_iterator c_tokentable_iter = tokentable_.begin();
	//while( tokentable_.end() != c_tokentable_iter
	//	|| TokenTableItem::PROCEDURE == c_tokentable_iter->itemtype_
	//	|| TokenTableItem::FUNCTION == c_tokentable_iter->itemtype_)
	//{
	//	++c_tokentable_iter;
	//}
	//// 第一个过程/函数的addr即为变量的存储空间（单位：4bytes）
	//int var_space = 0;
	//if(tokentable_.end() == c_tokentable_iter)	// 表示整个程序只有一个主函数
	//{
	//	// 如果有变量的话，找出最后一个变量，再计算地址
	//	// 如果没有变量，var_space保持为0
	//	if(tokentable_.size() != 0)	
	//	{
	//		const TokenTableItem &item = tokentable_.back();
	//		int base_addr = item.addr_;	// 最后一个变量的地址
	//		// 最后一个变量是数组的计算方式
	//		if(TokenTableItem::ARRAY == item.itemtype_)
	//		{
	//			var_space = base_addr + item.value_;
	//		}
	//		else	// 普通变量的计算方式
	//		{
	//			var_space = base_addr + 1;
	//		}
	//	}
	//}
	//else
	//{
	//	var_space = c_tokentable_iter->addr_;
	//}

	// TODO 二. 构造运行栈

	// TODO 三. 为变量在栈上分配空间

	// 四. 在四元式表中找到主函数的开始地址，对四元式进行汇编
	vector<Quaternary>::const_reverse_iterator rc_quaternary_iter = quaternarytable_.rbegin();
	while(rc_quaternary_iter != quaternarytable_.rend() 
		&& Quaternary::END != rc_quaternary_iter->type1_)
	{
		++rc_quaternary_iter;
	}
	// 从c_iter指向的四元式直到结束，都是属于主函数的四元式
	for(vector<Quaternary>::const_iterator c_quaternary_iter = rc_quaternary_iter.base();
		c_quaternary_iter != quaternarytable_.end(); ++c_quaternary_iter)
	{
		// TODO 转换汇编码
	}
}

// 普通函数的汇编过程
// var_space是该函数的局部变量的空间
// func_begin是四元式表中该函数的BEGIN语句的迭代器
void AssemblyMaker::OtherFunction(int var_space, vector<Quaternary>::const_iterator func_begin) throw()
{
	
}

// 普通过程的汇编过程
// var_space是该过程的局部变量的空间
// func_begin是四元式表中该过程的BEGIN语句的迭代器
void AssemblyMaker::OtherProcedure(int var_space, vector<Quaternary>::const_iterator proc_begin) throw()
{

}

// 返回过程/函数的局部变量所占的空间（单位：4bytes）
// c_iter为过程/函数在符号表中的位置的下一个位置
// 参数之所以这么要求，是因为主函数在符号表中没有位置，只能提供下一个位置
int AssemblyMaker::GetVariableSpace(TokenTable::const_iterator c_iter) const throw()
{
	// 如果该过程/函数没有局部变量
	if(tokentable_.end() == c_iter
		|| TokenTableItem::PROCEDURE == c_iter->itemtype_
		|| TokenTableItem::FUNCTION == c_iter->itemtype_)
	{
		return 0;
	}
	// 用while跳过当前过程/函数的局部变量
	while( tokentable_.end() != c_iter
		&& TokenTableItem::PROCEDURE != c_iter->itemtype_
		&& TokenTableItem::FUNCTION != c_iter->itemtype_)
	{
		++c_iter;
	}
	// 找出最后一个变量，得到其地址，再计算局部变量空间（地址+末变量空间）
	const TokenTableItem &item = (tokentable_.end() == c_iter) ? tokentable_.back() : *(c_iter - 1);
	// 最后一个变量可能是数组或普通变量，所以有两种不同的空间计算方式
	return item.addr_ + (TokenTableItem::ARRAY == item.itemtype_) ? item.value_ : 1;
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