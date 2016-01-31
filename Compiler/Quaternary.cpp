#include "Quaternary.h"

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


//const char* const Quaternary::OperandTypeString[] = {"", "", "", "TEMPORARY@"};
