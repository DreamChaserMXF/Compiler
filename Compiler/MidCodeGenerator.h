#ifndef MIDCODEGENERATOR_H
#define MIDCODEGENERATOR_H

#include <stack>
using std::stack;

#include "LexicalAnalyzer.h"
#include "TokenTable.h"
#include "Quaternary.h"
#include "ExpressionAttribute.h"



class MidCodeGenerator
{
public:
	MidCodeGenerator(LexicalAnalyzer &lexical_analyzer, const vector<string> &stringtable) throw();
	bool GenerateQuaternary() throw();
	TokenTable GetTokenTable() const throw();
	vector<Quaternary> GetQuaternaryTable() const throw();
	string toString() const throw();
	bool Print(const string &filename) const throw();
	void Print(std::ostream &output) const throw();
private:
	MidCodeGenerator(const MidCodeGenerator&) throw();
	
	void PrintFunctionFrame(const char *func_name, size_t depth) throw();// �������֡��Ϣ��depth���﷨����ʱ�ĺ����������


	void Routine(size_t depth) throw();				// ����
	void SubRoutine(size_t depth) throw();				// �ֳ���

	void ConstantPart(size_t depth) throw();			// ����˵������
	void constantDefination(size_t depth) throw();		// ��������

	void VariablePart(size_t depth) throw();			// ����˵������
	void VariableDefinition(size_t depth) throw();		// ��������
	void TypeSpecification(size_t depth) throw();		// ����

	void ProcedurePart(size_t depth) throw();			// ����˵������
	int  ProcedureHead(size_t depth) throw();			// �����ײ�
	void FunctionPart(size_t depth) throw();			// ����˵������
	int FunctionHead(size_t depth) throw();			// �����ײ�
	int ParameterList(size_t depth) throw();			// �βα� �����βθ���
	int ParameterTerm(size_t depth) throw();			// �βζ� �����βθ���

	void StatementBlockPart(size_t depth) throw();		// ������䲿��
	void Statement(size_t depth) throw();				// ���

	void AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth) throw();	// ��ֵ���

	ExpressionAttribute Expression(size_t depth) throw();	// ���ʽ�����ر��ʽ������
	ExpressionAttribute Term(size_t depth) throw();		// ��
	ExpressionAttribute Factor(size_t depth) throw();		// ����

	void IfStatement(size_t depth) throw();			// �������
	bool Condition(int label_positive, int label_negative, size_t depth) throw();// ����
	//void Condition(size_t depth) throw();		// ����
	bool BoolExpression(int label_positive, int label_negative, size_t depth) throw();	// �������ʽ
	bool BoolTerm(int label_positive, int label_negative, size_t depth) throw();		// ������
	bool BoolFactor(int label_positive, int label_negative, size_t depth) throw();		// ��������
	bool IsExpression(size_t depth) throw();	// �����Ƿ�Ϊ���ʽ�����ִʷ��������ֳ���
	bool ExpressionTest(size_t depth) throw();	// �����Ƿ�Ϊ���ʽ
	bool TermTest(size_t depth) throw();		// �����Ƿ�Ϊ��
	bool FactorTest(size_t depth) throw();		// �����Ƿ�Ϊ����
	bool ProcFuncCallStatementTest(size_t depth) throw();	// �����Ƿ�Ϊ����/�����������
	bool ArgumentListTest(size_t depth) throw();// �����Ƿ�Ϊʵ�α�

	void CaseStatement(size_t depth) throw();			// ������
	vector<int> CaseElement(int caselabel, int endlabel, size_t depth) throw();	// �����Ԫ��

	void ReadStatement(size_t depth) throw();			// �����
	void WriteStatement(size_t depth) throw();			// д���

	void WhileLoopStatement(size_t depth) throw();		// whileѭ�����
	void ForLoopStatement(size_t depth) throw();		// forѭ�����
	void ContinueStatement(size_t depth) throw();		// continue
	void BreakStatement(size_t depth) throw();			// break

	void ProcFuncCallStatement(const Token token_, const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw();	// �����������
	void ArgumentList(const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw();// ʵ�α�

	void SimplifyArrayOperand(ExpressionAttribute &attribute) throw();

	// ���ݳ�Ա
	LexicalAnalyzer &lexical_analyzer_;		// �󶨵Ĵʷ�������
	const vector<string> &stringtable_;		// �󶨵ĳ����ַ���
	TokenTable tokentable_;					// �󶨵ķ��ű�
	vector<Quaternary> quaternarytable_;	// ��Ԫʽ��

	Token token_;							// ��ǰ������token_
	int level_;								// ��ǰ�ķֳ����Ρ���ʼֵΪ1�����������Ϊ0�������Appendix 1��Ʊ�ע 15��
	int tempvar_index_;						// ��Ҫʹ�õ���һ����ʱ�������±�
	int label_index_;						// ��Ҫʹ�õ���һ��label���±�
	stack<int> continue_label_;				// continue������ת���ջ
	stack<int> break_label_;				// breal������ת���ջ

	bool is_successful_;					// �м���������Ƿ�ɹ�

	std::ostringstream generating_process_buffer_;	// �м�������ɹ��̵����
	std::string generating_format_string_;			// ��������﷨�������̵ı���
	vector<Token> tokenbuffer_;						// ������ű�ʱ���ڱ�������ص�token_
};


#endif
