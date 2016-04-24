#ifndef ASSEMBLYMAKER_H
#define ASSEMBLYMAKER_H

#include "TokenTable.h"
#include "Quaternary.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

class AssemblyMaker
{
public:
	AssemblyMaker(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, const vector<string> &stringtable) throw();
	void Print(std::ostream &out) const throw();
	bool Print(const string &filename) const throw();

	bool Assemble() throw();
private:
	AssemblyMaker(const AssemblyMaker &) throw();
	enum REGISTER{EAX, EBX, ECX, EDX};
	enum SINGLEOPERATOR{NEG, PUSH, IMUL, IDIV};
	enum DOUBLEOPERATOR{MOV, ADD, SUB, LEA, CMP};

	void Head() throw();
	void StackSegment() throw();
	void DataSegment() throw();
	void CodeBeginSegment() throw();
	void MainFunction() throw();
	void OtherFunction(TokenTable::const_iterator c_iter) throw();
	void EndStatement() throw();
	vector<Quaternary>::const_iterator GetProcFuncIterInQuaternaryTable(TokenTable::const_iterator c_iter) const throw();
	
	void TranslateQuaternary	(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 算术运算
	void TranslateNeg			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateAdd			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateSub			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateMul			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateDiv			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateAssign		(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateArrayAssign	(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 逻辑运算
	void TranslateJmp			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJe			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJne			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJg			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJng			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJl			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJnl			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateLabel			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 函数调用
	void TranslateSetP			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateSetRefP		(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateCall			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRet			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateStore			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRead			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateWrite			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 单操作数操作（NEG, PUSH, IMUL, IDIV）
	void OpGeneral	(enum SINGLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level) throw();
	void OpImmediate(enum SINGLEOPERATOR op,                                                int value)																														throw();
	void OpVar		(enum SINGLEOPERATOR op,                                                int tokentable_index,																	int para_num,                int level)	throw();
	void OpReference(enum SINGLEOPERATOR op,                                                int tokentable_index,																	int para_num,				 int level)	throw();
	void OpArray	(enum SINGLEOPERATOR op,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num,                int level) throw();
	void OpTemp		(enum SINGLEOPERATOR op,                                                int index,                                            int var_space)															throw();
	// 双操作数操作（第一个操作数是各种类型的内存变量，第二个是立即数或寄存器）（MOV, ADD, SUB, LEA）
	// OpGeneralRegister主要用在Store指令中，向内存中存数据用
	// OpGeneralImmediate主要用在1.对变量的立即数赋值；2. 对内存的直接的算术操作（所以其第二个操作数只能为立即数）。第2个功能往往在算术运算的四元式中化简后会用到。
	void OpGeneralRegister   (enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, enum REGISTER reg = EAX) throw();
	void OpGeneralImmediate  (enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, int immediate_value)	 throw();
	void OpVarRegister		 (enum DOUBLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level, enum REGISTER reg = EAX)												 throw();
	void OpVarImmediate		 (enum DOUBLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level, int immediate_value)													 throw();
	void OpReferenceRegister (enum DOUBLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level, enum REGISTER reg = EAX)												 throw();
	void OpReferenceImmediate(enum DOUBLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level, int immediate_value)													 throw();
	void OpArrayRegister	 (enum DOUBLEOPERATOR op,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num,                int level, enum REGISTER reg = EAX) throw();
	void OpArrayImmediate	 (enum DOUBLEOPERATOR op,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num,                int level, int immediate_value)	 throw();
	void OpTempRegister 	 (enum DOUBLEOPERATOR op,                                                int index,                                            int var_space           , enum REGISTER reg = EAX)												 throw();
	void OpTempImmediate	 (enum DOUBLEOPERATOR op,                                                int index,                                            int var_space           , int immediate_value)													 throw();
	// 双操作数操作（第一个操作数是寄存器，第二个操作数是各种类型的内存变量或立即数）（MOV, ADD, SUB, LEA）
	void OpRegisterGeneral	(enum DOUBLEOPERATOR op, enum REGISTER reg, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level)	throw();
	void OpRegisterImmediate(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int value)																														throw();
	void OpRegisterVar		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, int para_num,                                  int level)													throw();
	void OpRegisterReference(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, int para_num,                                  int level)													throw();
	void OpRegisterArray	(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num,                int level)	throw();
	void OpRegisterTemp		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int index,                                            int var_space)																throw();

	string FindExitLabel(vector<Quaternary>::const_iterator c_iter) throw();

	static string GenerateLabelString(int label_index);	// 通过label标号生成label字符串

	static const char * const RegisterName[4];	// 寄存器名
	static const char * const SingleOperatorName[4];
	static const char * const DoubleOperatorName[5];

	const vector<Quaternary> &quaternarytable_;
	const TokenTable &tokentable_;
	const vector<string> &stringtable_;
	std::ostringstream assemble_buffer;
};

#endif