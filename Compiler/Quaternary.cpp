#include "Quaternary.h"

const char* Quaternary::OPCodeString[] = {
			"ADD", "SUB", "MUL", "DIV", "ASG", "AASG",
			"JMP", "JE", "JNE", "JG", "JNG", "JL", "JNL",
			"CALL", "PUSHP", "RET",
			"BEGIN", "END", "LABEL"
};

//const char* const Quaternary::OperandTypeString[] = {"", "", "", "TEMPORARY@"};
