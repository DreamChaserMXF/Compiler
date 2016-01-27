#ifndef QUATERNARY_H
#define QUATERNARY_H

// 四元式类
// op src1 src2 dst
// 其中src1 src2 dst均有别称
// type1 type2 type3分别为src1, src2, dst的修饰类型
// type取值					操作数取值的意义
// IMMEDIATE_OPERAND		立即数的值
// CONSTANT_OPERAND			常数在符号表中的下标（以0起始）
// VARIABLE_OPERAND			变量在符号表中的下标（以0起始）
// TEMPORARY_OPERAND		变量在临时变量表中的序号（以0起始）
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