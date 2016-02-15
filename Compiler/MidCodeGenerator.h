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
	
	void PrintFunctionFrame(const char *func_name, size_t depth) throw();// 输出函数帧信息，depth是语法分析时的函数调用深度


	void Routine(size_t depth) throw();				// 程序
	void SubRoutine(size_t depth) throw();				// 分程序

	void ConstantPart(size_t depth) throw();			// 常量说明部分
	void constantDefination(size_t depth) throw();		// 常量定义

	void VariablePart(size_t depth) throw();			// 变量说明部分
	void VariableDefinition(size_t depth) throw();		// 变量定义
	void TypeSpecification(size_t depth) throw();		// 类型

	void ProcedurePart(size_t depth) throw();			// 过程说明部分
	int  ProcedureHead(size_t depth) throw();			// 过程首部
	void FunctionPart(size_t depth) throw();			// 函数说明部分
	int FunctionHead(size_t depth) throw();			// 函数首部
	int ParameterList(size_t depth) throw();			// 形参表 返回形参个数
	int ParameterTerm(size_t depth) throw();			// 形参段 返回形参个数

	void StatementBlockPart(size_t depth) throw();		// 复合语句部分
	void Statement(size_t depth) throw();				// 语句

	void AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth) throw();	// 赋值语句

	ExpressionAttribute Expression(size_t depth) throw();	// 表达式，返回表达式的类型
	ExpressionAttribute Term(size_t depth) throw();		// 项
	ExpressionAttribute Factor(size_t depth) throw();		// 因子

	void IfStatement(size_t depth) throw();			// 条件语句
	bool Condition(int label_positive, int label_negative, size_t depth) throw();// 条件
	//void Condition(size_t depth) throw();		// 条件
	bool BoolExpression(int label_positive, int label_negative, size_t depth) throw();	// 布尔表达式
	bool BoolTerm(int label_positive, int label_negative, size_t depth) throw();		// 布尔项
	bool BoolFactor(int label_positive, int label_negative, size_t depth) throw();		// 布尔因子
	bool IsExpression(size_t depth) throw();	// 测试是否为表达式（保持词法分析器现场）
	bool ExpressionTest(size_t depth) throw();	// 测试是否为表达式
	bool TermTest(size_t depth) throw();		// 测试是否为项
	bool FactorTest(size_t depth) throw();		// 测试是否为因子
	bool ProcFuncCallStatementTest(size_t depth) throw();	// 测试是否为过程/函数调用语句
	bool ArgumentListTest(size_t depth) throw();// 测试是否为实参表

	void CaseStatement(size_t depth) throw();			// 情况语句
	vector<int> CaseElement(int caselabel, int endlabel, size_t depth) throw();	// 情况表元素

	void ReadStatement(size_t depth) throw();			// 读语句
	void WriteStatement(size_t depth) throw();			// 写语句

	void WhileLoopStatement(size_t depth) throw();		// while循环语句
	void ForLoopStatement(size_t depth) throw();		// for循环语句
	void ContinueStatement(size_t depth) throw();		// continue
	void BreakStatement(size_t depth) throw();			// break

	void ProcFuncCallStatement(const Token token_, const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw();	// 函数调用语句
	void ArgumentList(const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw();// 实参表

	void SimplifyArrayOperand(ExpressionAttribute &attribute) throw();

	// 数据成员
	LexicalAnalyzer &lexical_analyzer_;		// 绑定的词法分析器
	const vector<string> &stringtable_;		// 绑定的常量字符表
	TokenTable tokentable_;					// 绑定的符号表
	vector<Quaternary> quaternarytable_;	// 四元式表

	Token token_;							// 当前解析的token_
	int level_;								// 当前的分程序层次。初始值为1。主函数层次为0。《详见Appendix 1设计备注 15》
	int tempvar_index_;						// 将要使用的下一个临时变量的下标
	int label_index_;						// 将要使用的下一个label的下标
	stack<int> continue_label_;				// continue语句的跳转标号栈
	stack<int> break_label_;				// breal语句的跳转标号栈

	bool is_successful_;					// 中间代码生成是否成功

	std::ostringstream generating_process_buffer_;	// 中间代码生成过程的输出
	std::string generating_format_string_;			// 辅助输出语法分析过程的变量
	vector<Token> tokenbuffer_;						// 插入符号表时用于保存多个相关的token_
};


#endif
