#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include "LexicalAnalyzer.h"
#include "TokenTable.h"
#include "Quaternary.h"
#include "ExpressionAttribute.h"

class SyntaxAnalyzer
{
public:
	SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer, const vector<string> &stringtable, TokenTable &tokentable, vector<Quaternary> &quaternarytable) throw();
	bool Parse() throw();
	string toString() const throw();
	bool Print(const string &filename) const throw();
	void Print(std::ostream &output) const throw();
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
	int  ProcedureHead(int depth) throw();			// �����ײ�
	void FunctionPart(int depth) throw();			// ����˵������
	int FunctionHead(int depth) throw();			// �����ײ�
	int ParameterList(int depth) throw();			// �βα� �����βθ���
	int ParameterTerm(int depth) throw();			// �βζ� �����βθ���
	vector<ExpressionAttribute> ArgumentList(int depth) throw();// ʵ�α�

	void StatementBlockPart(int depth) throw();		// ������䲿��
	void Statement(int depth) throw();				// ���

	void AssigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth) throw();	// ��ֵ���
	ExpressionAttribute Expression(int depth) throw();				// ���ʽ�����ر��ʽ������
	ExpressionAttribute Term(int depth) throw();					// ��
	ExpressionAttribute Factor(int depth) throw();					// ����
	void IfStatement(int depth) throw();			// �������
	void Condition(int label, int depth) throw();				// ����
	void CaseStatement(int depth) throw();			// ������
	void CaseList(int depth) throw();				// �����Ԫ��
	void ReadStatement(int depth) throw();			// �����
	void WriteStatement(int depth) throw();			// д���
	void ForLoopStatement(int depth) throw();		// forѭ�����
	void ProcedureCallStatement(const Token token_, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth) throw();// ���̵������
	void FunctionCallStatement(const Token token_, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth) throw();	// �����������


	LexicalAnalyzer &lexical_analyzer_;		// �󶨵Ĵʷ�������
	TokenTable &tokentable_;				// �󶨵ķ��ű�
	const vector<string> &stringtable_;		// �󶨵ĳ����ַ���
	vector<Quaternary> &quaternarytable_;		// ��Ԫʽ��

	Token token_;							// ��ǰ������token_
	int level_;								// ��ǰ�ķֳ����Ρ���ʼֵΪ0
//	int quaternary_index;					// ��һ�����ɵ���Ԫʽ���±꣨��ʼΪ0��
	int tempvar_index_;						// ��Ҫʹ�õ���һ����ʱ�������±�
	int label_index_;						// ��Ҫʹ�õ���һ��label���±�

	bool is_successful_;					// �﷨�����Ƿ�ɹ�

	std::ostringstream syntax_info_buffer_;	// �﷨�������̵����
	std::string syntax_assist_buffer_;		// ��������﷨�������̵ı���
	vector<Token> tokenbuffer_;				// ������ű�ʱ���ڱ�������ص�token_
};

#endif