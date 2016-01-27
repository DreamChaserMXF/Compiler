#ifndef TOKENTABLE_H
#define TOKENTABLE_H
#include <ostream>
#include <vector>
#include <stack>
#include "Token.h"
#include "TokenTableItem.h"

using std::vector;
using std::stack;

// ���ʣ�����ඨ��const���������𣿱Ͼ�������ָ����const����ѽ
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
	bool SearchDefinitionInCurrentLevel(const string &name) throw();		// �����ڵ�ǰ�ӳ������Ƿ���ڶ��壨���ڳ�/����������䣩
	
	iterator SearchDefinition(const Token &token) throw();					// ���Ҷ��崦
	const_iterator end() const throw();
//	TokenTableItem::DecorateType AddUsedLine(const Token &token);	// �Ը�����token�����䶨�崦�����ӵ�ǰ�е�������Ϣ�������ظ�token����������
	
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
	void Print(const string &fileName) const;	// ������ļ�
	void Print(std::ostream &output) const;		// �������


private:
	TokenTable(const TokenTable&) throw();	// ���ø��ƹ��캯��
	const_iterator SearchDefinition(const Token &token) const throw();	// ���Ҷ��崦

	vector<TokenTableItem> rows_;	// ���ű��ÿһ��
	stack<int> subroutine_tokentableindex_stack_;		// ���ű��зֳ��������±�ջ
	int addr_;
	stack<int> subroutine_tokentableaddr_stack_;			// ���ű��зֳ������ڵ�ַջ
};

#endif