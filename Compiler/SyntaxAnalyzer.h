#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include <stack>
using std::stack;

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
	vector<TokenTableItem::DecorateType> ArgumentList(size_t depth) throw();// ʵ�α�

	void StatementBlockPart(size_t depth) throw();		// ������䲿��
	void Statement(size_t depth) throw();				// ���

	void AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth) throw();	// ��ֵ���

	ExpressionAttribute Expression(size_t depth) throw();	// ���ʽ�����ر��ʽ������
	ExpressionAttribute Term(size_t depth) throw();		// ��
	ExpressionAttribute Factor(size_t depth) throw();		// ����

	void IfStatement(size_t depth) throw();			// �������
	void Condition(int endlabel, size_t depth) throw();// ����
	void CaseStatement(size_t depth) throw();			// ������
	vector<int> CaseElement(int caselabel, int endlabel, size_t depth) throw();	// �����Ԫ��
	void ReadStatement(size_t depth) throw();			// �����
	void WriteStatement(size_t depth) throw();			// д���
	void WhileLoopStatement(size_t depth) throw();		// whileѭ�����
	void ForLoopStatement(size_t depth) throw();		// forѭ�����
	void ContinueStatement(size_t depth) throw();	// continue
	void BreakStatement(size_t depth) throw();		// break
	void ProcedureCallStatement(const Token token_, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, size_t depth) throw();	// ���̵������
	void FunctionCallStatement(const Token token_, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, size_t depth) throw();	// �����������

	void SimplifyArrayOperand(ExpressionAttribute &attribute) throw();
//	void SetTempVarCount(int proc_func_index, int max_tempvar_count) throw();

	// ���ݳ�Ա
	LexicalAnalyzer &lexical_analyzer_;		// �󶨵Ĵʷ�������
	TokenTable &tokentable_;				// �󶨵ķ��ű�
	const vector<string> &stringtable_;		// �󶨵ĳ����ַ���
	vector<Quaternary> &quaternarytable_;	// ��Ԫʽ��

	Token token_;							// ��ǰ������token_
	int level_;								// ��ǰ�ķֳ����Ρ���ʼֵΪ1�����������Ϊ0�������Appendix 1��Ʊ�ע 15��
	int tempvar_index_;						// ��Ҫʹ�õ���һ����ʱ�������±�
//	int max_local_temp_count_;				// ������ʱ��������(���캯����ÿ��END��Ԫʽ����ʱ�ж�Ҫ��ʼ��Ϊ0)
	int label_index_;						// ��Ҫʹ�õ���һ��label���±�
	stack<int> continue_label_;	// continue������ת���ջ
	stack<int> break_label_;		// breal������ת���ջ

	bool is_successful_;					// �﷨�����Ƿ�ɹ�

	std::ostringstream syntax_info_buffer_;	// �﷨�������̵����
	std::string syntax_assist_buffer_;		// ��������﷨�������̵ı���
	vector<Token> tokenbuffer_;				// ������ű�ʱ���ڱ�������ص�token_
};

#endif