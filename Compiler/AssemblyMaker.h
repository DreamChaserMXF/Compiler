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

	void Head() throw();
	void StackSegment() throw();
	void DataSegment() throw();
	void CodeBeginSegment() throw();
	void MainFunction() throw();
	void OtherFunction(TokenTable::const_iterator c_iter) throw();
	void EndStatement() throw();
	vector<Quaternary>::const_iterator GetProcFuncIterInQuaternaryTable(TokenTable::const_iterator c_iter) const throw();
	
	void TranslateQuaternary(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 算术运算
	void TranslateNeg(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateAdd(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateSub(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateMul(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateDiv(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateArrayAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 逻辑运算
	void TranslateJmp(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJe(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJne(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJg(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJng(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJl(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateJnl(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateLabel(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 函数调用
	void TranslateSetP(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateCall(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRet(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateStore(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateRead(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	void TranslateWrite(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw();
	// 装载操作
	void LoadGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level, enum REGISTER reg, bool load_addr = false) throw();
	void LoadImmediate(int value, enum REGISTER reg) throw();
	void LoadVar(int tokentable_index, int para_num, int level, enum REGISTER reg, bool load_addr = false) throw();
	void LoadArray(int tokentable_index, int array_offset, int para_num, int level, enum REGISTER reg, bool load_addr = false) throw();
	void LoadTemp(int index, int var_space, enum REGISTER reg) throw();
	// 存储操作
	void StoreGeneral(Quaternary::AddressingMethod addressingmethod, int tokentable_index, int array_offset, int para_num, int var_space, int level) throw();
	void StoreVar(int tokentable_index, int para_num, int level) throw();
	void StoreArray(int tokentable_index, int array_offset, int para_num, int level) throw();
	void StoreTemp(int index, int var_space) throw();
	// 压栈操作
	void PushGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level) throw();
	void PushImmediate(int value) throw();
	void PushVar(int tokentable_index, int para_num, int level) throw();
	void PushArray(int tokentable_index, int array_offset, int para_num, int level) throw();
	void PushTemp(int index, int var_space) throw();
	

	string FindExitLabel(vector<Quaternary>::const_iterator c_iter) throw();

	static string LabelStringFormat(int label_index);	// 通过label标号生成label字符串

	static const char * const RegisterName[3];	// 寄存器名

	const vector<Quaternary> &quaternarytable_;
	const TokenTable &tokentable_;
	const vector<string> &stringtable_;
	std::ostringstream assemble_buffer;
};

#endif