#ifndef TOKENTABLE_H
#define TOKENTABLE_H

#pragma warning(disable:4290)

#include <ostream>
#include <vector>
#include <stack>

#include "Token.h"
#include "TokenTableItem.h"
#include "ExpressionAttribute.h"
using std::vector;
using std::stack;

// 疑问：这个类定义const函数有用吗？毕竟不会出现该类的const对象呀
// 答：会出现const对象，例如在AssemblyMaker类中就有const引用
class TokenTable
{
public:
	typedef vector<TokenTableItem>::iterator iterator;
	typedef vector<TokenTableItem>::const_iterator const_iterator;
	typedef vector<TokenTableItem>::reverse_iterator reverse_iterator;
	typedef vector<TokenTableItem>::const_reverse_iterator const_reverse_iterator;

	TokenTable();

	string toString() const throw();
	void Print(const string &fileName) const throw();	// 输出到文件
	void Print(std::ostream &output) const throw();		// 输出到流

	void clear();
	TokenTableItem at(int index) const throw(std::out_of_range);
	// 定位与重定位
	void Locate();
	void Relocate();

	bool IsInLocalActiveScope(const string &name) throw();		// 查找在当前子程序中是否存在定义（用于常/变量定义语句）
	iterator SearchDefinition(const Token &token) throw();		// 查找定义处
	const_iterator begin() const throw();
	const_iterator end() const throw();
	//const TokenTableItem& back() const throw();
	size_t size() const throw();
	
	vector<ExpressionAttribute> GetProcFuncParameterAttributes(const_iterator iter) throw();
	void AddConstItem(Token constIdentifier, TokenTableItem::DecorateType decoratetype_, int value, int level) throw();
	void AddVariableItem(Token variableIdentifier, TokenTableItem::DecorateType decoratetype_, int level) throw();
	void AddArrayItem(Token arrayIdentifier, TokenTableItem::DecorateType decoratetype_, int arrayLength, int level) throw();
	int  AddProcedureItem(Token procedureIdentifier, int level) throw();
	int  AddFunctionItem(Token functionIdentifier, int level) throw();
	void SetParameterCount(const string &proc_func_name, int parameterCount) throw();
	void SetFunctionReturnType(const string &func_name, TokenTableItem::DecorateType decoratetype_) throw();
	void AddParameterItem(Token parameterIdentifier, TokenTableItem::DecorateType decoratetype_, bool isref, int level) throw();

	// 汇编过程中需要
	int GetVariableSpace(TokenTable::const_iterator c_iter) const throw();
	int GetParameterNum(int var_index) const throw();
private:
	const_iterator SearchDefinition(const Token &token) const throw();	// 查找定义处
	vector<TokenTableItem> rows_;	// 符号表的每一行
	stack<int> subroutine_tokentableindex_stack_;		// 符号表中分程序的入口下标栈
	int addr_;
	stack<int> subroutine_tokentableaddr_stack_;		// 符号表中分程序的入口地址栈
};

#endif