#ifndef SYNTAXEXCEPTION_H
#define SYNTAXEXCEPTION_H

//#pragma warning (disable : 4996)

#include <exception>
#include <sstream>

/*
// �ó�����Ϣ������������ʼ��һ���쳣����
// ����what()��ȡ�쳣��˵����Ϣ
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