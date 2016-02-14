#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

#pragma warning(disable:4290)	// warning of exception specification(VS2010 not support)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include "Token.h"
#include "LexException.h"

using std::string;
using std::vector;
using std::map;
using std::ostream;
using std::ifstream;
using std::set;

// 词法分析类
// 最容易出现的BUG是while循环中，若ch='\0'，会不会无限循环
class LexicalAnalyzer
{
public:
	LexicalAnalyzer(const string &srcFileName) throw();
	bool IsBound() const throw();						// 是否绑定了源文件
	bool Parse() throw();								// 进行词法分析
	bool Print(string fileName) const throw();			// 输出到文件
	void Print(ostream &output) const throw();			// 输出到流
	void ResetTokenPos() throw();						// 重置符号vector的迭代器token_vector_的位置
	bool GetNextToken(Token &token) throw();			// 获取下一符号，成功则返回true，读到符号尾则返回false
	string GetLine(size_t line_no) throw();
	vector<string> getStringTable() const throw();		// 获得常量字符串表
private:
	LexicalAnalyzer(const LexicalAnalyzer&) throw();	// 声明private的复制构造函数，但并不实现==>禁用复制
	char getNextChar(bool skipSpace = false) throw();	// 得到下一个字符，参数表示是否跳过空字符
	void ParseCurrentToken(Token &token, char &ch) throw(LexException);			// 根据已读到的当前字符，分析完整个token，并读入下一个字符
	// 下面的几个handle函数，在确定了token之后，均要读入下一个字符
	void SingleLineCommentHandle(Token &token, char &ch) throw();				// 单行注释
	void BlockCommentHandle		(Token &token, char &ch) throw(LexException);	// 多行注释
	void DigitHandle			(Token &token, char &ch) throw(LexException);	// 处理无符号整数常量
	void LetterHandle			(Token &token, char &ch) throw();				// 处理字母
	void StringHandle			(Token &token, char &ch) throw(LexException);	// 处理字符串常量
	void CharHandle				(Token &token, char &ch) throw(LexException);	// 处理字符常量
	void ColonHandle			(Token &token, char &ch) throw();				// 处理冒号
	void LessthanHandle			(Token &token, char &ch) throw();				// 处理小于号
	void GreaterthanHandle		(Token &token, char &ch) throw();				// 处理大于号
	void PlusHandle				(Token &token, char &ch) throw(LexException);	// 处理加号(单操作符或双操作符)，注意，这里的token要存储上一个识别出的单词
	void MinusHandle			(Token &token, char &ch) throw(LexException);	// 处理减号(单操作符或双操作符)，注意，这里的token要存储上一个识别出的单词

	ifstream srcfile_;							// 输入的文件流
	int currentline_;							// 当前处理的行号（从1开始计数）
	vector<Token> token_vector_;				// 存放Token的数组
	vector<Token>::const_iterator token_iter_;	// 遍历token_vector_的迭代器
	vector<string> code_lines_;					// 代码行
	set<string> string_set;						// 常量字符串表

	static const char escape_sequence[];
};

#endif