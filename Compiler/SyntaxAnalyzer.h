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
	
	void PrintFunctionFrame(const char *func_name, int depth);// 输出函数帧信息，depth是语法分析时的函数调用深度
	void routine(int depth);					// 程序
	void subRoutine(int depth);				// 分程序

	void constantPart(int depth);			// 常量说明部分
	void constantDefination(int depth);		// 常量定义

	void variablePart(int depth);			// 变量说明部分
	void variableDefinition(int depth);		// 变量定义
	void typeSpecification(int depth);		// 类型

	void procedurePart(int depth);			// 过程说明部分
	void procedureHead(int depth);			// 过程首部
	void functionPart(int depth);			// 函数说明部分
	void functionHead(int depth);			// 函数首部
	int parameterList(int depth);			// 形参表 返回形参个数
	int parameterTerm(int depth);			// 形参段 返回形参个数
	vector<TokenTableItem::DecorateType> argumentList(int depth);// 实参表

	void statementBlockPart(int depth);		// 复合语句部分
	void statement(int depth);				// 语句

	void assigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth);	// 赋值语句
	TokenTableItem::DecorateType expression(int depth);				// 表达式，返回表达式的类型
	TokenTableItem::DecorateType term(int depth);					// 项
	TokenTableItem::DecorateType factor(int depth);					// 因子
	void ifStatement(int depth);			// 条件语句
	void condition(int depth);				// 条件
	void caseStatement(int depth);			// 情况语句
	void caseList(int depth);				// 情况表元素
	void readStatement(int depth);			// 读语句
	void writeStatement(int depth);			// 写语句
	void forLoopStatement(int depth);		// for循环语句
	void procedureCallStatement(const Token token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth);// 过程调用语句
	void functionCallStatement(const Token token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth);	// 函数调用语句


	LexicalAnalyzer &lexicalAnalyzer;	// 绑定的词法分析器
	TokenTable &tokenTable;				// 绑定的符号表
	std::ostream &output;				// 绑定语法分析过程输出流
	Token token;						// 当前解析的token
	int level;							// 当前的分程序层次。初始值为0	
	vector<Token> tokenBuffer;			// 插入符号表时用于保存多个相关的token
	bool isSuccessful;					// 语法分析是否成功
};

#endif