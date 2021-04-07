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

// ���ʣ�����ඨ��const���������𣿱Ͼ�������ָ����const����ѽ
// �𣺻����const����������AssemblyMaker���о���const����
class TokenTable
{
public:
	typedef vector<TokenTableItem>::iterator iterator;
	typedef vector<TokenTableItem>::const_iterator const_iterator;
	typedef vector<TokenTableItem>::reverse_iterator reverse_iterator;
	typedef vector<TokenTableItem>::const_reverse_iterator const_reverse_iterator;

	TokenTable();

	string toString() const throw();
	void Print(const string &fileName) const throw();	// ������ļ�
	void Print(std::ostream &output) const throw();		// �������

	void clear();
	TokenTableItem at(int index) const throw(std::out_of_range);
	// ��λ���ض�λ
	void Locate();
	void Relocate();

	bool IsInLocalActiveScope(const string &name) throw();		// �����ڵ�ǰ�ӳ������Ƿ���ڶ��壨���ڳ�/����������䣩
	iterator SearchDefinition(const Token &token) throw();		// ���Ҷ��崦
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

	// ����������Ҫ
	int GetVariableSpace(TokenTable::const_iterator c_iter) const throw();
	int GetParameterNum(int var_index) const throw();
private:
	const_iterator SearchDefinition(const Token &token) const throw();	// ���Ҷ��崦
	vector<TokenTableItem> rows_;	// ���ű��ÿһ��
	stack<int> subroutine_tokentableindex_stack_;		// ���ű��зֳ��������±�ջ
	int addr_;
	stack<int> subroutine_tokentableaddr_stack_;		// ���ű��зֳ������ڵ�ַջ
};

#endif