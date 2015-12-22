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
	bool SearchDefinitionInCurrentLevel(const string &name);		// 查找在当前子程序中是否存在定义（用于常/变量定义语句）
	
	iterator SearchDefinition(const Token &token);					// 查找定义处
	const_iterator end() const;
//	TokenTableItem::DecorateType AddUsedLine(const Token &token);	// 对给定的token，在其定义处，增加当前行的引用信息，并返回该token的修饰类型
	
//	vector<TokenTableItem::DecorateType> GetProcFuncParameter(const Token &token);
	vector<TokenTableItem::DecorateType> GetProcFuncParameter(const_iterator iter);
	void AddConstItem(Token constIdentifier, TokenTableItem::DecorateType decorateType, int value, int level);
	void AddVariableItem(Token variableIdentifier, TokenTableItem::DecorateType decorateType, int level);
	void AddArrayItem(Token arrayIdentifier, TokenTableItem::DecorateType decorateType, int arrayLength, int level);
	void AddProcedureItem(Token procedureIdentifier, int level);
	void AddFunctionItem(Token functionIdentifier, int level);
	void SetParameterCount(const string &proc_func_name, int parameterCount);
	void SetFunctionReturnType(const string &func_name, TokenTableItem::DecorateType decorateType);
	void AddParameterItem(Token parameterIdentifier, TokenTableItem::DecorateType decorateType, int level);
	
	string toString() const;
	void Print(const string &fileName) const;	// 输出到文件
	void Print(std::ostream &output) const;		// 输出到流


private:
	TokenTable(const TokenTable&);	// 禁用复制构造函数
	const_iterator SearchDefinition(const Token &token) const;	// 查找定义处

	vector<TokenTableItem> rows;	// 符号表的每一行
	stack<int> subRoutineStack;		// 符号表中分程序的入口下标栈
	int addr;
	stack<int> addrStack;			// 符号表中分程序的入口地址栈
};

#endif