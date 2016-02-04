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
	// ��������
	void TranslateNeg			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateAdd			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateSub			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateMul			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateDiv			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateAssign		(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateArrayAssign	(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// �߼�����
	void TranslateJmp			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJe			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJne			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJg			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJng			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJl			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJnl			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateLabel			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// ��������
	void TranslateSetP			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateCall			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRet			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateStore			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRead			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateWrite			(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// ��������������NEG, PUSH, IMUL, IDIV��
	void OpGeneral	(enum SINGLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level) throw();
	void OpImmediate(enum SINGLEOPERATOR op,                                                int value)																														throw();
	void OpVar		(enum SINGLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level)													throw();
	void OpArray	(enum SINGLEOPERATOR op,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset,                              int level) throw();
	void OpTemp		(enum SINGLEOPERATOR op,                                                int index,                                            int var_space)															throw();
	// ˫��������������һ���������Ǹ������͵��ڴ�������ڶ�������������Ĵ�������MOV, ADD, SUB, LEA��
	// OpGeneralRegister��Ҫ����Storeָ���У����ڴ��д�������
	// OpGeneralImmediate��Ҫ����1.�Ա�������������ֵ��2. ���ڴ��ֱ�ӵ�����������������ڶ���������ֻ��Ϊ������������2�����������������������Ԫʽ�л������õ���
	void OpGeneralRegister  (enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, enum REGISTER reg = EAX) throw();
	void OpGeneralImmediate (enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, int immediate_value)	 throw();
	void OpVarRegister		(enum DOUBLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level, enum REGISTER reg = EAX)												 throw();
	void OpVarImmediate		(enum DOUBLEOPERATOR op,                                                int tokentable_index, int para_num,                                  int level, int immediate_value)													 throw();
	void OpArrayRegister	(enum DOUBLEOPERATOR op,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset,                              int level, enum REGISTER reg = EAX) throw();
	void OpArrayImmediate	(enum DOUBLEOPERATOR op,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset,                              int level, int immediate_value)	 throw();
	void OpTempRegister 	(enum DOUBLEOPERATOR op,                                                int index,                                            int var_space           , enum REGISTER reg = EAX)												 throw();
	void OpTempImmediate	(enum DOUBLEOPERATOR op,                                                int index,                                            int var_space           , int immediate_value)													 throw();
	// ˫��������������һ���������ǼĴ������ڶ����������Ǹ������͵��ڴ����������������MOV, ADD, SUB, LEA��
	void OpRegisterGeneral	(enum DOUBLEOPERATOR op, enum REGISTER reg, Quaternary::AddressingMethod addressingmethod, int index_or_value,   Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level)	throw();
	void OpRegisterImmediate(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int value)																														throw();
	void OpRegisterVar		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, int para_num,                                  int level)													throw();
	void OpRegisterArray	(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset,                              int level)	throw();
	void OpRegisterTemp		(enum DOUBLEOPERATOR op, enum REGISTER reg,                                                int index,                                            int var_space)																throw();

	string FindExitLabel(vector<Quaternary>::const_iterator c_iter) throw();

	static string GenerateLabelString(int label_index);	// ͨ��label�������label�ַ���

	static const char * const RegisterName[4];	// �Ĵ�����
	static const char * const SingleOperatorName[4];
	static const char * const DoubleOperatorName[4];

	const vector<Quaternary> &quaternarytable_;
	const TokenTable &tokentable_;
	const vector<string> &stringtable_;
	std::ostringstream assemble_buffer;
};

#endif