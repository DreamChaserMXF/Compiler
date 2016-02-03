#ifndef TOKENTABLEEXCEPTION_H
#define TOKENTABLEEXCEPTION_H
	
#include "Token.h"

// �ó�����Ϣ������������ʼ��һ�����ű��쳣����
// ����what()��ȡ�쳣��˵����Ϣ
class TokenTableException : public std::exception
{
public:
	TokenTableException(const char *error_message, const Token &error_token) throw() : exception(), error_message_()
	{
		std::ostringstream buf;
		buf << "line " << error_token.lineNumber_ << ": " << error_token.toString() << '\t' << error_message;
		error_message_ = buf.str();
	}
	virtual const char* what() const throw()
	{
		return error_message_.c_str();
	}
private:
	string error_message_;
};
#endif