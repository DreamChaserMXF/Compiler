#include "Quaternary.h"
#include <assert.h>

const char* Quaternary::OPCodeString[] = {
			"NIL_OP",
			"NEG", "ADD", "SUB", "MUL", "DIV", "ASG", "AASG", "STORE",
			"JMP", "JE", "JNE", "JG", "JNG", "JL", "JNL",
			"FUNC_CALL", "PROC_CALL", "READ", "WRITE", "SETP", "SETREFP", "RET",
			"BEGIN", "END", "LABEL"
};

Quaternary::Quaternary() throw() 
		: op_(NIL_OP), method1_(NIL_ADDRESSING), src1_(0), method2_(NIL_ADDRESSING), src2_(0), method3_(NIL_ADDRESSING), dst_(0)
{}
Quaternary::Quaternary(OPCode op, AddressingMethod method1, int src1, AddressingMethod method2, int src2, AddressingMethod method3, int dst) throw()
	: op_(op), method1_(method1), src1_(src1), method2_(method2), src2_(src2), method3_(method3), dst_(dst), dst_decoratetype_(TokenTableItem::VOID)
{}

void Quaternary::UpdateTempVarSpace(std::vector<Quaternary> &quaternarytable) throw()
{
	std::vector<Quaternary>::iterator iter = quaternarytable.begin();
	assert(Quaternary::BEGIN == iter->op_);
	FindTempVar(iter);
}
// 从一个函数的BEGIN语句的迭代器开始，寻找这个函数用到的临时变量的个数，并更新到BEGIN语句中
// 最后返回指向该函数结束后的下一条语句的迭代器
// 如果这个函数有嵌套的函数，则对嵌套函数的BEGIN语句进行同样的更新
// 注意，该函数要求四元式中的BEGIN与END一定要匹配
std::vector<Quaternary>::iterator Quaternary::FindTempVar(const std::vector<Quaternary>::iterator &begin_iter) throw()
{
	std::vector<Quaternary>::iterator next_iter = begin_iter + 1;
	while(Quaternary::BEGIN == next_iter->op_)
	{
		next_iter = FindTempVar(next_iter);
	}
	// 现在next_iter指向该函数的BEGIN语句之后的第一条属于该函数的语句
	int max_temp_index = -1;
	while(Quaternary::END != next_iter->op_)
	{
		// 在已有的逻辑下，如果申请了新的临时变量，则一定会用在dst操作数上
		// 所以前两个if语句并不是必要的
		//if(Quaternary::TEMPORARY_ADDRESSING == next_iter->method1_
		//	&& next_iter->src1_ > max_temp_index)
		//{
		//	max_temp_index = next_iter->src1_;
		//}
		//if(Quaternary::TEMPORARY_ADDRESSING == next_iter->method2_
		//	&& next_iter->src2_ > max_temp_index)
		//{
		//	max_temp_index = next_iter->src2_;
		//}
		if(Quaternary::TEMPORARY_ADDRESSING == next_iter->method3_
			&& next_iter->dst_ > max_temp_index)
		{
			max_temp_index = next_iter->dst_;
		}
		++next_iter;
	}
	// 更新BEGIN语句
	begin_iter->src2_ = max_temp_index + 1;
	// 返回指向函数结束后的下一个语句的迭代器
	return ++next_iter;
}

// 通过过程/函数在四元式表中的BEGIN语句的迭代器，找到其过程/函数体的第一条语句的迭代器
// 这里要求过程/函数的BEGIN和END一定要配对
std::vector<Quaternary>::const_iterator Quaternary::GetFunctionBody(std::vector<Quaternary>::const_iterator begin_iter) throw()
{
	++begin_iter;
	if(Quaternary::BEGIN != begin_iter->op_)
	{
		return begin_iter;
	}

	int func_num = 1;	// 已经读到了一个begin
	while(func_num > 0 || Quaternary::BEGIN == (begin_iter+1)->op_)
	{
		++begin_iter;
		if(Quaternary::BEGIN == begin_iter->op_)
		{
			++func_num;
		}
		else if(Quaternary::END == begin_iter->op_)
		{
			--func_num;
		}
	}
	return ++begin_iter;
}