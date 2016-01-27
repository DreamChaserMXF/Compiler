#ifndef SYNTAXEXCEPTION_H
#define SYNTAXEXCEPTION_H

//#pragma warning (disable : 4996)

#include <exception>
#include <sstream>

/*
// 用出错信息、出错单词来初始化一个异常对象
// 调用what()获取异常的说明信息
class SyntaxTokenException : public std::exception
{
public:
	SyntaxTokenException(const char *errorMessage, const Token &errToken) : exception(), errMsg()
	{
		std::ostringstream buf;
//		buf << "line " << errToken.lineNumber_ << '\t' << errToken.toString() << '\t' << errorMessage;
		buf << "line " << errToken.lineNumber_ << ": " << errToken.toString() << '\t' << errorMessage;
		errMsg = buf.str();
	}
	virtual const char* what() const
	{
		return errMsg.c_str();
	}
private:
	string errMsg;
};
*/

#endif