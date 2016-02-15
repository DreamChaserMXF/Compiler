#ifndef SEMANTICSANALYZER_H
#define SEMANTICSANALYZER_H

#include "LexicalAnalyzer.h"
#include "TokenTable.h"

class SemanticsAnalyzer
{
public:
	SemanticsAnalyzer(LexicalAnalyzer &lexical_analyzer) throw();
	bool Parse() throw();
	TokenTable GetTokenTable() const throw();
	string toString() const throw();
	bool Print(const string &filename) const throw();
	void Print(std::ostream &output) const throw();
private:
	enum ErrorType{
		REDEFINITION, UNDEFINITION, ARGUMENTNUMBERNOTMATCH, WRONGTYPE, OUTERCONTINUE, OUTERBREAK,
	};
	
	SemanticsAnalyzer(const SemanticsAnalyzer&) throw();
	
	void PrintFunctionFrame(const char *func_name, size_t depth) throw();// �������֡��Ϣ��depth���﷨����ʱ�ĺ����������

	void Routine(size_t depth) throw();					// ����
	void SubRoutine(size_t depth) throw();				// �ֳ���

	void ConstantPart(size_t depth) throw();			// ����˵������
	void constantDefination(size_t depth) throw();		// ��������

	void VariablePart(size_t depth) throw();			// ����˵������
	void VariableDefinition(size_t depth) throw();		// ��������
	void TypeSpecification(size_t depth) throw();		// ����

	void ProcedurePart(size_t depth) throw();			// ����˵������
	void ProcedureHead(size_t depth) throw();			// �����ײ�
	void FunctionPart(size_t depth) throw();			// ����˵������
	void FunctionHead(size_t depth) throw();			// �����ײ�
	int ParameterList(size_t depth) throw();			// �βα� �����βθ���
	int ParameterTerm(size_t depth) throw();			// �βζ� �����βθ���

	void StatementBlockPart(size_t depth) throw();		// ������䲿��
	void Statement(size_t depth) throw();				// ���

	void AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth) throw();	// ��ֵ���
	TokenTableItem::DecorateType Expression(size_t depth) throw();	// ���ʽ�����ر��ʽ������
	TokenTableItem::DecorateType Term(size_t depth) throw();		// ��
	TokenTableItem::DecorateType Factor(size_t depth) throw();		// ����

	void IfStatement(size_t depth) throw();		// �������
	void Condition(size_t depth) throw();		// ����
	void BoolExpression(size_t depth) throw();	// �������ʽ
	void BoolTerm(size_t depth) throw();		// ������
	void BoolFactor(size_t depth) throw();		// ��������
	bool IsExpression(size_t depth) throw();	// �����Ƿ�Ϊ���ʽ�����ִʷ��������ֳ���
	bool ExpressionTest(size_t depth) throw();	// �����Ƿ�Ϊ���ʽ
	bool TermTest(size_t depth) throw();		// �����Ƿ�Ϊ��
	bool FactorTest(size_t depth) throw();		// �����Ƿ�Ϊ����
	bool ProcFuncCallStatementTest(size_t depth) throw();	// �����Ƿ�Ϊ����/�����������
	bool ArgumentListTest(size_t depth) throw();// �����Ƿ�Ϊʵ�α�

	void CaseStatement(size_t depth) throw();			// ������
	void CaseElement(size_t depth) throw();	// �����Ԫ��

	void WhileLoopStatement(size_t depth) throw();		// whileѭ�����
	void ForLoopStatement(size_t depth) throw();		// forѭ�����
	void ContinueStatement(size_t depth) throw();		// continue
	void BreakStatement(size_t depth) throw();			// break

	void ProcFuncCallStatement(const Token token_, const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw();	// �����������
	void ArgumentList(const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw();// ʵ�α�

	void ReadStatement(size_t depth) throw();			// �����
	void WriteStatement(size_t depth) throw();			// д���

	void ErrorHandle(ErrorType error_type, const char *errinfo = NULL) throw();

	// ���ݳ�Ա
	LexicalAnalyzer &lexical_analyzer_;				// �󶨵Ĵʷ�������
	TokenTable tokentable_;							// �󶨵ķ��ű�

	Token token_;									// ��ǰ������token_
	int level_;										// ��ǰ�ķֳ����Ρ���ʼֵΪ1���������������Ϊ0�����Appendix 1��Ʊ�ע 15����
	int loop_level;									// continue��break���������飨�Ƿ���ѭ�����ڣ�

	bool is_successful_;							// ��������Ƿ�ɹ�

	std::ostringstream semantics_process_buffer_;	// ����������̵��������
	std::string semantics_format_string_;			// ����������̵�����ĸ�ʽ����
	vector<Token> tokenbuffer_;						// ������ű�ʱ���ڱ�������ص�token_
};

#endif