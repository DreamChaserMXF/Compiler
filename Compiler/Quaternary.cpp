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
// ��һ��������BEGIN���ĵ�������ʼ��Ѱ����������õ�����ʱ�����ĸ����������µ�BEGIN�����
// ��󷵻�ָ��ú������������һ�����ĵ�����
// ������������Ƕ�׵ĺ��������Ƕ�׺�����BEGIN������ͬ���ĸ���
// ע�⣬�ú���Ҫ����Ԫʽ�е�BEGIN��ENDһ��Ҫƥ��
std::vector<Quaternary>::iterator Quaternary::FindTempVar(const std::vector<Quaternary>::iterator &begin_iter) throw()
{
	std::vector<Quaternary>::iterator next_iter = begin_iter + 1;
	while(Quaternary::BEGIN == next_iter->op_)
	{
		next_iter = FindTempVar(next_iter);
	}
	// ����next_iterָ��ú�����BEGIN���֮��ĵ�һ�����ڸú��������
	int max_temp_index = -1;
	while(Quaternary::END != next_iter->op_)
	{
		// �����е��߼��£�����������µ���ʱ��������һ��������dst��������
		// ����ǰ����if��䲢���Ǳ�Ҫ��
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
	// ����BEGIN���
	begin_iter->src2_ = max_temp_index + 1;
	// ����ָ�������������һ�����ĵ�����
	return ++next_iter;
}

// ͨ������/��������Ԫʽ���е�BEGIN���ĵ��������ҵ������/������ĵ�һ�����ĵ�����
// ����Ҫ�����/������BEGIN��ENDһ��Ҫ���
std::vector<Quaternary>::const_iterator Quaternary::GetFunctionBody(std::vector<Quaternary>::const_iterator begin_iter) throw()
{
	++begin_iter;
	if(Quaternary::BEGIN != begin_iter->op_)
	{
		return begin_iter;
	}

	int func_num = 1;	// �Ѿ�������һ��begin
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