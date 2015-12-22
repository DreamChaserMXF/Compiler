#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include "LexicalAnalyzer.h"
#include "TokenTable.h"

class SyntaxAnalyzer
{
public:
	SyntaxAnalyzer(LexicalAnalyzer &lexicalAnalyzer_, TokenTable &tokenTable_, std::ostream &output_ = std::cout);
	bool Parse();
private:
	SyntaxAnalyzer(const SyntaxAnalyzer&);
	
	void PrintFunctionFrame(const char *func_name, int depth);// �������֡��Ϣ��depth���﷨����ʱ�ĺ����������
	void routine(int depth);					// ����
	void subRoutine(int depth);				// �ֳ���

	void constantPart(int depth);			// ����˵������
	void constantDefination(int depth);		// ��������

	void variablePart(int depth);			// ����˵������
	void variableDefinition(int depth);		// ��������
	void typeSpecification(int depth);		// ����

	void procedurePart(int depth);			// ����˵������
	void procedureHead(int depth);			// �����ײ�
	void functionPart(int depth);			// ����˵������
	void functionHead(int depth);			// �����ײ�
	int parameterList(int depth);			// �βα� �����βθ���
	int parameterTerm(int depth);			// �βζ� �����βθ���
	vector<TokenTableItem::DecorateType> argumentList(int depth);// ʵ�α�

	void statementBlockPart(int depth);		// ������䲿��
	void statement(int depth);				// ���

	void assigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth);	// ��ֵ���
	TokenTableItem::DecorateType expression(int depth);				// ���ʽ�����ر��ʽ������
	TokenTableItem::DecorateType term(int depth);					// ��
	TokenTableItem::DecorateType factor(int depth);					// ����
	void ifStatement(int depth);			// �������
	void condition(int depth);				// ����
	void caseStatement(int depth);			// ������
	void caseList(int depth);				// �����Ԫ��
	void readStatement(int depth);			// �����
	void writeStatement(int depth);			// д���
	void forLoopStatement(int depth);		// forѭ�����
	void procedureCallStatement(const Token token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth);// ���̵������
	void functionCallStatement(const Token token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth);	// �����������


	LexicalAnalyzer &lexicalAnalyzer;	// �󶨵Ĵʷ�������
	TokenTable &tokenTable;				// �󶨵ķ��ű�
	std::ostream &output;				// ���﷨�������������
	Token token;						// ��ǰ������token
	int level;							// ��ǰ�ķֳ����Ρ���ʼֵΪ0	
	vector<Token> tokenBuffer;			// ������ű�ʱ���ڱ�������ص�token
	bool isSuccessful;					// �﷨�����Ƿ�ɹ�
};

#endif