#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include "Token.h"

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
	LexicalAnalyzer(const string &srcFileName);
	bool Parse();							// 进行词法分析
	void Print(string fileName) const;		// 输出到文件
	void Print(ostream &output) const;		// 输出到流
	void ResetTokenPos();					// 重置符号vector的迭代器tokenVector的位置
	bool GetNextToken(Token &token);		// 获取下一符号，成功则返回true，读到符号尾则返回false
	vector<string> getStringTable();		// 获得常量字符串表
private:
	LexicalAnalyzer(const LexicalAnalyzer&);		// 声明private的复制构造函数，但并不实现==>禁用复制
	char getNextChar(bool skipSpace = false);		// 得到下一个字符，参数表示是否跳过空字符
	void parseCurrentToken(Token &token, char &ch);	// 根据已读到的当前字符，分析完整个token，并读入下一个字符
	// 下面的几个handle函数，在确定了token之后，均要读入下一个字符
	void singleLineCommentHandle(Token &token, char &ch);			// 单行注释
	void blockCommentHandle		(Token &token, char &ch);			// 多行注释
	void digitHandle			(Token &token, char &ch);	// 处理无符号整数常量
	void letterHandle			(Token &token, char &ch);	// 处理字母
	void stringHandle			(Token &token, char &ch);	// 处理字符串常量
	void charHandle				(Token &token, char &ch);	// 处理字符常量
	void colonHandle			(Token &token, char &ch);	// 处理冒号
	void ltHandle				(Token &token, char &ch);	// 处理小于号
	void gtHandle				(Token &token, char &ch);	// 处理大于号
	void plusHandle				(Token &token, char &ch);	// 处理加号(单操作符或双操作符)，注意，这里的token要存储上一个识别出的单词
	void minusHandle			(Token &token, char &ch);	// 处理减号(单操作符或双操作符)，注意，这里的token要存储上一个识别出的单词

	ifstream srcFile;						// 输入的文件流
	int currentLine;						// 当前处理的行号
	vector<Token> tokenVector;				// 存放Token的数组
	vector<Token>::const_iterator tokenIter;
	set<string> stringSet;				// 常量字符串表
};

#endif