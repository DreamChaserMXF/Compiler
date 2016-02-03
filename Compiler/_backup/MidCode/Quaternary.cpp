#include "Quaternary.h"
#include <assert.h>

const char* Quaternary::OPCodeString[] = {
			"NIL_OP",
			"NEG", "ADD", "SUB", "MUL", "DIV", "ASG", "AASG", "STORE",
			"JMP", "JE", "JNE", "JG", "JNG", "JL", "JNL",
			"FUNC_CALL", "PROC_CALL", "READ", "WRITE", "SETP", "RET",
			"BEGIN", "END", "LABEL"
};

Quaternary::Quaternary() throw() 
		: op_(NIL_OP), type1_(NIL_OPERAND), src1_(0), type2_(NIL_OPERAND), src2_(0), type3_(NIL_OPERAND), dst_(0)
{}
Quaternary::Quaternary(OPCode op, OperandType type1, int src1, OperandType type2, int src2, OperandType type3, int dst) throw()
	: op_(op), type1_(type1), src1_(src1), type2_(type2), src2_(src2), type3_(type3), dst_(dst)
{}

void Quaternary::UpdateTempVarSpace(std::vector<Quaternary> &quaternarytable) throw()
{
	std::vector<Quaternary>::iterator iter = quaternarytable.begin();
	assert(Quaternary::BEGIN == iter->op_);
	FindTempVar(iter, quaternarytable.end());
}
// ��һ��������BEGIN���ĵ�������ʼ��Ѱ����������õ�����ʱ�����ĸ����������µ�BEGIN�����
// ��󷵻�ָ��ú������������һ�����ĵ�����
// end_iter������ʹû���ҵ�END��䣬ҲӦ��ֹͣ�ĵ�����λ��
// ������������Ƕ�׵ĺ��������Ƕ�׺�����BEGIN������ͬ���ĸ���
std::vector<Quaternary>::iterator Quaternary::FindTempVar(const std::vector<Quaternary>::iterator &begin_iter,
	const std::vector<Quaternary>::const_iterator &end_iter) throw()
{
	std::vector<Quaternary>::iterator next_iter = begin_iter + 1;
	while(Quaternary::BEGIN == next_iter->op_)
	{
		next_iter = FindTempVar(next_iter, end_iter);
	}
	// ����next_iterָ��ú�����BEGIN���֮��ĵ�һ�����ڸú��������
	int max_temp_index = -1;
	do
	{
		// �����е��߼��£�����������µ���ʱ��������һ��������dst��������
		// ����ǰ����if��䲢���Ǳ�Ҫ��
		//if(Quaternary::TEMPORARY_OPERAND == next_iter->type1_
		//	&& next_iter->src1_ > max_temp_index)
		//{
		//	max_temp_index = next_iter->src1_;
		//}
		//if(Quaternary::TEMPORARY_OPERAND == next_iter->type2_
		//	&& next_iter->src2_ > max_temp_index)
		//{
		//	max_temp_index = next_iter->src2_;
		//}
		if(Quaternary::TEMPORARY_OPERAND == next_iter->type3_
			&& next_iter->dst_ > max_temp_index)
		{
			max_temp_index = next_iter->dst_;
		}
		++next_iter;
	} while(Quaternary::END != next_iter->op_);
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
	while(func_num > 0)
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