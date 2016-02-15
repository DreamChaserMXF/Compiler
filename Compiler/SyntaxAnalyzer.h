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

	enum ErrorType{
		LACKENDINGPERIOD, WRONGENDINGTOKEN, REDUNDANTTOOKEN, NOSTATEMENTBLOCK, LACKSEMICOLON, LACKIDENTIFIER, LACKEQU,
		LACKCONSTANT, LACKTYPECOLON, LACKCASECOLON, LACKLEFTBRACKET, LACKRIGHTBRACKET, LACKLEFTPAREN, LACKRIGHTPAREN, LACKCONSTINTEGER, LACKOF, 
		LACKRWTYPE, LACKASSIGNINGTOKEN, WRONGFACTOR, LACKTHEN, LACKLOGICOPERATOR, LACKCONSTANTORIDENTIFIER, LACKDO, LACKTO_DOWNTO,
	};

	SyntaxAnalyzer(const SyntaxAnalyzer&) throw();
	
	void PrintFunctionFrame(const char *func_name, size_t depth) throw();// 输出函数帧信息，depth是语法分析时的函数调用深度

	void Routine(size_t depth) throw();				// 程序
	void SubRoutine(size_t depth) throw();				// 分程序

	void ConstantPart(size_t depth) throw();			// 常量说明部分
	void constantDefination(size_t depth) throw();		// 常量定义

	void VariablePart(size_t depth) throw();			// 变量说明部分
	void VariableDefinition(size_t depth) throw();		// 变量定义
	void TypeSpecification(size_t depth) throw();		// 类型

	void ProcedurePart(size_t depth) throw();			// 过程说明部分
	void ProcedureHead(size_t depth) throw();			// 过程首部
	void FunctionPart(size_t depth) throw();			// 函数说明部分
	void FunctionHead(size_t depth) throw();			// 函数首部
	void ParameterList(size_t depth) throw();			// 形参表
	void ParameterTerm(size_t depth) throw();			// 形参段

	void StatementBlockPart(size_t depth) throw();		// 复合语句部分
	void Statement(size_t depth) throw();				// 语句

	void AssigningStatement(size_t depth) throw();	// 赋值语句

	void Expression(size_t depth) throw();		// 表达式
	void Term(size_t depth) throw();			// 项
	void Factor(size_t depth) throw();			// 因子

	void IfStatement(size_t depth) throw();		// 条件语句
	void Condition(size_t depth) throw();		// 条件
	void BoolExpression(size_t depth) throw();	// 布尔表达式
	void BoolTerm(size_t depth) throw();		// 布尔项
	void BoolFactor(size_t depth) throw();		// 布尔因子
	bool IsExpression(size_t depth) throw();	// 测试是否为表达式（保持词法分析器现场）
	bool ExpressionTest(size_t depth) throw();	// 测试是否为表达式
	bool TermTest(size_t depth) throw();		// 测试是否为项
	bool FactorTest(size_t depth) throw();		// 测试是否为因子
	bool ProcFuncCallStatementTest(size_t depth) throw();	// 测试是否为过程/函数调用语句
	bool ArgumentListTest(size_t depth) throw();// 测试是否为实参表

	void CaseStatement(size_t depth) throw();			// 情况语句
	void CaseElement(size_t depth) throw();	// 情况表元素

	void ReadStatement(size_t depth) throw();			// 读语句
	void WriteStatement(size_t depth) throw();			// 写语句

	void WhileLoopStatement(size_t depth) throw();		// while循环语句
	void ForLoopStatement(size_t depth) throw();		// for循环语句
	void ContinueStatement(size_t depth) throw();		// continue
	void BreakStatement(size_t depth) throw();			// break

	void ProcFuncCallStatement(size_t depth) throw();	// 过程/函数调用语句
	void ArgumentList(size_t depth) throw();// 实参表

	void ErrorHandle(ErrorType error_type) throw();

	// 数据成员
	LexicalAnalyzer &lexical_analyzer_;		// 绑定的词法分析器

	Token token_;							// 当前解析的token_
	bool is_successful_;					// 语法分析是否成功

	std::ostringstream syntax_process_buffer_;	// 语法分析过程的输出
	std::string syntax_format_string_;		// 辅助输出语法分析过程的变量
};

#endif