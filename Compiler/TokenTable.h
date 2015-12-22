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
	bool SearchDefinitionInCurrentLevel(const string &name);		// �����ڵ�ǰ�ӳ������Ƿ���ڶ��壨���ڳ�/����������䣩
	
	iterator SearchDefinition(const Token &token);					// ���Ҷ��崦
	const_iterator end() const;
//	TokenTableItem::DecorateType AddUsedLine(const Token &token);	// �Ը�����token�����䶨�崦�����ӵ�ǰ�е�������Ϣ�������ظ�token����������
	
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
	void Print(const string &fileName) const;	// ������ļ�
	void Print(std::ostream &output) const;		// �������


private:
	TokenTable(const TokenTable&);	// ���ø��ƹ��캯��
	const_iterator SearchDefinition(const Token &token) const;	// ���Ҷ��崦

	vector<TokenTableItem> rows;	// ���ű��ÿһ��
	stack<int> subRoutineStack;		// ���ű��зֳ��������±�ջ
	int addr;
	stack<int> addrStack;			// ���ű��зֳ������ڵ�ַջ
};

#endif