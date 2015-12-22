#ifndef LEXEXCEPTION_H
#define LEXEXCEPTION_H

//#pragma warning (disable : 4996)

#include <exception>
#include <sstream>
#include <cctype>

class LexExceptionDEPRECATED : public std::exception
{
public:
	LexExceptionDEPRECATED(const char *errMsg):exception(errMsg){}
};

// 用出错信息、出错字符及出错行号来初始化一个异常对象
// 调用what()获取异常的说明信息
class LexException : public std::exception
{
public:
	LexException(const char *errorMessage, char errCh, int lineNo) : exception(), errMsg()
	{
		std::ostringstream buf;
		buf << "line " << lineNo << ": \'";
		if(isprint(errCh))
		{
			buf << errCh;
		}
		else
		{
			buf << '\\' << (int)errCh;
		}
		buf << "\': " << errorMessage;
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