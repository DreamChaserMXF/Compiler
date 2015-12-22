#ifndef TOKENTABLEEXCEPTION_H
#define TOKENTABLEEXCEPTION_H
	
#include "Token.h"

// �ó�����Ϣ������������ʼ��һ�����ű��쳣����
// ����what()��ȡ�쳣��˵����Ϣ
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