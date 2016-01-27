#ifndef QUATERNARY_H
#define QUATERNARY_H

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
	
	enum OPCode{ADD, SUB, MUL, DIV, ASG, AASG,
				JMP, JE, JNE, JG, JNG, JL, JNL,
				CALL, PUSHP, RET,
				BEGIN, END, LABEL,};
	enum OperandType{IMMEDIATE_OPERAND, CONSTANT_OPERAND, VARIABLE_OPERAND, TEMPORARY_OPERAND, };
	
	Quaternary(OPCode op, OperandType type1, int src1, OperandType type2, int src2, OperandType type3, int dst) throw()
		: op_(op), type1_(type1), src1_(src1), type2_(type2), src2_(src2), type3_(type3), dst_(dst)
	{}

	OPCode op_;

	OperandType type1_;
	union{
		int src1_;
		int procedure_;
		int function_;
	};

	OperandType type2_;
	union
	{
		int src2_;
		int offset_;
	};

	OperandType type3_;
	union
	{
		int dst_;
		int label_;
		int para_num_;
		int index_;
		int label_num_;
	};

	static const char* OPCodeString[];
//	static const char* const OperandTypeString[];

};


#endif