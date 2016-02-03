#ifndef QUATERNARY_H
#define QUATERNARY_H

#include <vector>

// ��Ԫʽ��
// op src1 src2 dst
// ����src1 src2 dst���б��
// type1 type2 type3�ֱ�Ϊsrc1, src2, dst����������
// typeȡֵ					������ȡֵ������
// IMMEDIATE_OPERAND		��������ֵ
// CONSTANT_OPERAND			�����ڷ��ű��е��±꣨��0��ʼ��
// VARIABLE_OPERAND			�����ڷ��ű��е��±꣨��0��ʼ��
// TEMPORARY_OPERAND		��������ʱ�������е���ţ���0��ʼ��
// 
class Quaternary
{
public:
	
	enum OPCode{NIL_OP = 0, NEG, ADD, SUB, MUL, DIV, ASG, AASG, STORE,
				JMP, JE, JNE, JG, JNG, JL, JNL,
				FUNC_CALL, PROC_CALL, READ, WRITE, SETP, RET,
				BEGIN, END, LABEL,};
	// ȥ���˳�����������CONSTANT_OPERAND by mxf at 15:24 1/30/2016
	enum OperandType{NIL_OPERAND = 0, IMMEDIATE_OPERAND, STRING_OPERAND, VARIABLE_OPERAND, ARRAY_OPERAND, TEMPORARY_OPERAND, LABEL_OPERAND, PROC_FUNC_INDEX, PARANUM_OPERAND, };
	
	Quaternary() throw();
	Quaternary(OPCode op, OperandType type1, int src1, OperandType type2, int src2, OperandType type3, int dst) throw();


	static void UpdateTempVarSpace(std::vector<Quaternary> &quaternarytable) throw();
	static std::vector<Quaternary>::iterator Quaternary::FindTempVar(
								const std::vector<Quaternary>::iterator &begin_iter, 
								const std::vector<Quaternary>::const_iterator &end_iter) throw();
	static std::vector<Quaternary>::const_iterator GetFunctionBody(std::vector<Quaternary>::const_iterator begin_iter) throw();

	OPCode op_;
	
	//OperandType type1_;
	//int src1_;

	//OperandType type2_;
	//int src2_;

	//OperandType type3_;
	//int dst_;

//	class{int i; char c;};
	OperandType type1_;
	union{
		int src1_;
		int para_num_;
	};

	OperandType type2_;
	union
	{
		int src2_;
		int offset2_;
	};

	OperandType type3_;
	union
	{
		int dst_;
		int label3_;
		int para_num3_;
		int index3_;
		int label_num3_;
	};

	static const char* OPCodeString[];
//	static const char* const OperandTypeString[];


};

#endif