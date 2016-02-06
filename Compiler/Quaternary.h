#ifndef QUATERNARY_H
#define QUATERNARY_H

#include "TokenTableItem.h"
#include <vector>

// ��Ԫʽ��
// op src1 src2 dst
// ����src1 src2 dst���б��
// type1 type2 type3�ֱ�Ϊsrc1, src2, dst����������
// typeȡֵ										������ȡֵ������
// IMMEDIATE_ADDRESSING							��������ֵ
// CONSTANT_ADDRESSING							�����ڷ��ű��е��±꣨��0��ʼ��
// VARIABLE_ADDRESSING, REFERENCE_ADDRESSING	�����ڷ��ű��е��±꣨��0��ʼ��
// TEMPORARY_ADDRESSING							��������ʱ�������е���ţ���0��ʼ��
// 
class Quaternary
{
public:
	
	enum OPCode{NIL_OP = 0, NEG, ADD, SUB, MUL, DIV, ASG, AASG, STORE,
				JMP, JE, JNE, JG, JNG, JL, JNL,
				FUNC_CALL, PROC_CALL, READ, WRITE, SETP, SETREFP, RET,
				BEGIN, END, LABEL,};
	// ȥ���˳�����������CONSTANT_ADDRESSING by mxf at 15:24 1/30/2016
	enum AddressingMethod{NIL_ADDRESSING = 0, IMMEDIATE_ADDRESSING, STRING_ADDRESSING, VARIABLE_ADDRESSING, ARRAY_ADDRESSING, 
		TEMPORARY_ADDRESSING, REFERENCE_ADDRESSING,};
	
	Quaternary() throw();
	Quaternary(OPCode op, AddressingMethod method1, int src1, AddressingMethod method2, int src2, AddressingMethod method3, int dst) throw();


	static void UpdateTempVarSpace(std::vector<Quaternary> &quaternarytable) throw();
	static std::vector<Quaternary>::iterator Quaternary::FindTempVar(const std::vector<Quaternary>::iterator &begin_iter) throw();
	static std::vector<Quaternary>::const_iterator GetFunctionBody(std::vector<Quaternary>::const_iterator begin_iter) throw();

	OPCode op_;
	
	AddressingMethod method1_;
	int src1_;

	AddressingMethod method2_;
	union
	{
		int src2_;
		int offset2_;
	};

	AddressingMethod method3_;
	int dst_;

	TokenTableItem::DecorateType dst_decoratetype_;

	static const char* OPCodeString[];
//	static const char* const OperandTypeString[];


};

#endif