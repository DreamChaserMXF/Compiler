#ifndef TOKENTABLE_H
#define TOKENTABLE_H
#include <ostream>
#include <vector>
#include <stack>
#include "Token.h"
#include "TokenTableItem.h"

using std::vector;
using std::stack;

// 疑问：这个类定义const函数有用吗？毕竟不会出现该类的const对象呀
class TokenTable
{
public:
	typedef vector<TokenTableItem>::iterator iterator;
	typedef vector<TokenTableItem>::const_iterator const_iterator;
	typedef vector<TokenTableItem>::reverse_iterator reverse_iterator;
	typedef vector<TokenTableItem>::const_reverse_iterator const_reverse_iterator;

	TokenTable();
	void Locate();
	void Relocate();

	//bool isConst(const Token &token) const;
	bool SearchDefinitionInCurrentLevel(const string &name) throw();		// 查找在当前子程序中是否存在定义（用于常/变量定义语句）
	
	iterator SearchDefinition(const Token &token) throw();					// 查找定义处
	const_iterator end() const throw();
//	TokenTableItem::DecorateType AddUsedLine(const Token &token);	// 对给定的token，在其定义处，增加当前行的引用信息，并返回该token的修饰类型
	
	vector<TokenTableItem::DecorateType> GetProcFuncParameter(const_iterator iter) throw();
	void AddConstItem(Token constIdentifier, TokenTableItem::DecorateType decoratetype_, int value, int level) throw();
	void AddVariableItem(Token variableIdentifier, TokenTableItem::DecorateType decoratetype_, int level) throw();
	void AddArrayItem(Token arrayIdentifier, TokenTableItem::DecorateType decoratetype_, int arrayLength, int level) throw();
	void AddProcedureItem(Token procedureIdentifier, int level) throw();
	void AddFunctionItem(Token functionIdentifier, int level) throw();
	void SetParameterCount(const string &proc_func_name, int parameterCount) throw();
	void SetFunctionReturnType(const string &func_name, TokenTableItem::DecorateType decoratetype_) throw();
	void AddParameterItem(Token parameterIdentifier, TokenTableItem::DecorateType decoratetype_, int level) throw();
	
	string toString() const;
	void Print(const string &fileName) const;	// 输出到文件
	void Print(std::ostream &output) const;		// 输出到流


private:
	TokenTable(const TokenTable&) throw();	// 禁用复制构造函数
	const_iterator SearchDefinition(const Token &token) const throw();	// 查找定义处

	vector<TokenTableItem> rows_;	// 符号表的每一行
	stack<int> subroutine_tokentableindex_stack_;		// 符号表中分程序的入口下标栈
	int addr_;
	stack<int> subroutine_tokentableaddr_stack_;			// 符号表中分程序的入口地址栈
};

#endif