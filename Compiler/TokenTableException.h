#ifndef TOKENTABLEEXCEPTION_H
#define TOKENTABLEEXCEPTION_H
	
#include "Token.h"

// 用出错信息、出错单词来初始化一个符号表异常对象
// 调用what()获取异常的说明信息
class TokenTableException : public std::exception
{
public:
	TokenTableException(const char *errorMessage, const Token &errToken) : exception(), errMsg()
	{
		std::ostringstream buf;
//		buf << "line " << errToken.toString() << '\t' << errorMessage;
		buf << "line " << errToken.lineNumber << ": " << errToken.toString() << '\t' << errorMessage;
		errMsg = buf.str();
	}
	virtual const char* what() const
	{
		return errMsg.c_str();
	}
private:
	string errMsg;
};
#endif