#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include "LexicalAnalyzer.h"
#include "TokenTable.h"

class SyntaxAnalyzer
{
public:
	SyntaxAnalyzer(LexicalAnalyzer &lexicalAnalyzer_, TokenTable &tokenTable_) throw();
	bool Parse() throw();
private:
	SyntaxAnalyzer(const SyntaxAnalyzer&) throw();
	
	void PrintFunctionFrame(const char *func_name, int depth) throw();// �������֡��Ϣ��depth���﷨����ʱ�ĺ����������
	void Routine(int depth) throw();				// ����
	void SubRoutine(int depth) throw();				// �ֳ���

	void ConstantPart(int depth) throw();			// ����˵������
	void constantDefination(int depth) throw();		// ��������

	void VariablePart(int depth) throw();			// ����˵������
	void VariableDefinition(int depth) throw();		// ��������
	void TypeSpecification(int depth) throw();		// ����

	void ProcedurePart(int depth) throw();			// ����˵������
	void ProcedureHead(int depth) throw();			// �����ײ�
	void FunctionPart(int depth) throw();			// ����˵������
	void FunctionHead(int depth) throw();			// �����ײ�
	int ParameterList(int depth) throw();			// �βα� �����βθ���
	int ParameterTerm(int depth) throw();			// �βζ� �����βθ���
	vector<TokenTableItem::DecorateType> ArgumentList(int depth) throw();// ʵ�α�

	void StatementBlockPart(int depth) throw();		// ������䲿��
	void Statement(int depth) throw();				// ���

	void AssigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth) throw();	// ��ֵ���
	TokenTableItem::DecorateType Expression(int depth) throw();				// ���ʽ�����ر��ʽ������
	TokenTableItem::DecorateType Term(int depth) throw();					// ��
	TokenTableItem::DecorateType Factor(int depth) throw();					// ����
	void IfStatement(int depth) throw();			// �������
	void Condition(int depth) throw();				// ����
	void CaseStatement(int depth) throw();			// ������
	void CaseList(int depth) throw();				// �����Ԫ��
	void ReadStatement(int depth) throw();			// �����
	void WriteStatement(int depth) throw();			// д���
	void ForLoopStatement(int depth) throw();		// forѭ�����
	void ProcedureCallStatement(const Token token_, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth) throw();// ���̵������
	void FunctionCallStatement(const Token token_, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth) throw();	// �����������


	LexicalAnalyzer &lexical_analyzer_;	// �󶨵Ĵʷ�������
	TokenTable &tokentable_;				// �󶨵ķ��ű�
	std::ostringstream stringbuffer_;	// ���﷨�������������
	Token token_;						// ��ǰ������token_
	int level_;							// ��ǰ�ķֳ����Ρ���ʼֵΪ0	
	vector<Token> tokenbuffer_;			// ������ű�ʱ���ڱ�������ص�token_
	bool is_successful_;					// �﷨�����Ƿ�ɹ�
};

#endif