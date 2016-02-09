#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include "LexicalAnalyzer.h"
#include "TokenTable.h"

class SyntaxAnalyzer
{
public:
	SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer) throw();
	bool Parse() throw();
	string toString() const throw();
	bool Print(const string &filename) const throw();
	void Print(std::ostream &output) const throw();
private:
	SyntaxAnalyzer(const SyntaxAnalyzer&) throw();
	
	void PrintFunctionFrame(const char *func_name, size_t depth) throw();// �������֡��Ϣ��depth���﷨����ʱ�ĺ����������

	void Routine(size_t depth) throw();				// ����
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
	void ParameterList(size_t depth) throw();			// �βα�
	void ParameterTerm(size_t depth) throw();			// �βζ�
	void ArgumentList(size_t depth) throw();// ʵ�α�

	void StatementBlockPart(size_t depth) throw();		// ������䲿��
	void Statement(size_t depth) throw();				// ���

	void AssigningStatement(size_t depth) throw();	// ��ֵ���

	void Expression(size_t depth) throw();	// ���ʽ�����ر��ʽ������
	void Term(size_t depth) throw();		// ��
	void Factor(size_t depth) throw();		// ����

	void IfStatement(size_t depth) throw();			// �������
	void Condition(size_t depth) throw();// ����

	void CaseStatement(size_t depth) throw();			// ������
	void CaseElement(size_t depth) throw();	// �����Ԫ��

	void ReadStatement(size_t depth) throw();			// �����
	void WriteStatement(size_t depth) throw();			// д���

	void WhileLoopStatement(size_t depth) throw();		// whileѭ�����
	void ForLoopStatement(size_t depth) throw();		// forѭ�����
	void ContinueStatement(size_t depth) throw();		// continue
	void BreakStatement(size_t depth) throw();			// break

	void ProcFuncCallStatement(size_t depth) throw();	// ����/�����������


	// ���ݳ�Ա
	LexicalAnalyzer &lexical_analyzer_;		// �󶨵Ĵʷ�������

	Token token_;							// ��ǰ������token_
	bool is_successful_;					// �﷨�����Ƿ�ɹ�

	std::ostringstream syntax_process_buffer_;	// �﷨�������̵����
	std::string syntax_format_string_;		// ��������﷨�������̵ı���
};

#endif