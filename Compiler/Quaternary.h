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
	
	enum OPCode{OP_NIL = 0, NEG, ADD, SUB, MUL, DIV, ASG, AASG,
				JMP, JE, JNE, JG, JNG, JL, JNL,
				FUNC_CALL, PROC_CALL, READ, WRITE, SETP, RET,
				BEGIN, END, LABEL,};
	enum OperandType{OPERAND_NIL = 0, IMMEDIATE_OPERAND, CONSTANT_OPERAND, STRING_OPERAND, VARIABLE_OPERAND, ARRAY_OPERAND, TEMPORARY_OPERAND, LABEL_OPERAND, PROC_FUNC_INDEX, PARANUM_OPERAND, };
	
	Quaternary() throw();
	Quaternary(OPCode op, OperandType type1, int src1, OperandType type2, int src2, OperandType type3, int dst) throw();

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