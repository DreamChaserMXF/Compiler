#ifndef QUATERNARY_H
#define QUATERNARY_H

#include "TokenTableItem.h"
#include <vector>

// 四元式类
// op src1 src2 dst
// 其中src1 src2 dst均有别称
// type1 type2 type3分别为src1, src2, dst的修饰类型
// type取值										操作数取值的意义
// IMMEDIATE_ADDRESSING							立即数的值
// CONSTANT_ADDRESSING							常数在符号表中的下标（以0起始）
// VARIABLE_ADDRESSING, REFERENCE_ADDRESSING	变量在符号表中的下标（以0起始）
// TEMPORARY_ADDRESSING							变量在临时变量表中的序号（以0起始）
// 
class Quaternary
{
public:
	
	enum OPCode{NIL_OP = 0, NEG, ADD, SUB, MUL, DIV, ASG, AASG, STORE,
				JMP, JE, JNE, JG, JNG, JL, JNL,
				FUNC_CALL, PROC_CALL, READ, WRITE, SETP, SETREFP, RET,
				BEGIN, END, LABEL,};
	// 去掉了常变量操作数CONSTANT_ADDRESSING by mxf at 15:24 1/30/2016
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