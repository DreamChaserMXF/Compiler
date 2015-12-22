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

// �ó�����Ϣ�������ַ��������к�����ʼ��һ���쳣����
// ����what()��ȡ�쳣��˵����Ϣ
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