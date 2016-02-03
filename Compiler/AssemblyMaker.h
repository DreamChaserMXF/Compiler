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
	enum REGISTER{EAX, EBX, EDX};
	enum SINGLEOPERATOR{PUSH, IMUL, IDIV};
	enum DOUBLEOPERATOR{MOV, ADD, SUB, LEA};

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
	void TranslateCall			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRet			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateStore			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRead			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateWrite			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 存储操作
	void StoreGeneral	(Quaternary::AddressingMethod addressingmethod, int tokentable_index, int array_offset, int para_num, int var_space, int level) throw();
	void StoreVar		(int tokentable_index,                                                                  int para_num,                int level) throw();
	void StoreArray		(int tokentable_index,                                                int array_offset,                              int level) throw();
	void StoreTemp		(int index,                                                                                           int var_space)            throw();
	// 单操作数操作
	void SingleOperation2General	(enum SINGLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   int array_offset, int para_num, int var_space, int level) throw();
	void SingleOperation2Immediate	(enum SINGLEOPERATOR op,                                                int value) throw();
	void SingleOperation2Var		(enum SINGLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level) throw();
	void SingleOperation2Array		(enum SINGLEOPERATOR op,                                                int tokentable_index, int array_offset,                              int level) throw();
	void SingleOperation2Temp		(enum SINGLEOPERATOR op,                                                int index,                                            int var_space)            throw();
	// 双操作数操作
	void DoubleOperation2General	(enum DOUBLEOPERATOR op, enum REGISTER reg, Quaternary::AddressingMethod addressingmethod, int index_or_value,   int array_offset, int para_num, int var_space, int level) throw();
	void DoubleOperation2Immediate	(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int value) throw();
	void DoubleOperation2Var		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, int para_num,                                  int level) throw();
	void DoubleOperation2Array		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, int array_offset,                              int level) throw();
	void DoubleOperation2Temp		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int index,                                            int var_space)            throw();
	

	string FindExitLabel(vector<Quaternary>::const_iterator c_iter) throw();

	static string GenerateLabelString(int label_index);	// 通过label标号生成label字符串

	static const char * const RegisterName[3];	// 寄存器名
	static const char * const SingleOperatorName[3];
	static const char * const DoubleOperatorName[4];

	const vector<Quaternary> &quaternarytable_;
	const TokenTable &tokentable_;
	const vector<string> &stringtable_;
	std::ostringstream assemble_buffer;
};

#endif