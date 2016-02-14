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

// �ʷ�������
// �����׳��ֵ�BUG��whileѭ���У���ch='\0'���᲻������ѭ��
class LexicalAnalyzer
{
public:
	LexicalAnalyzer(const string &srcFileName) throw();
	bool IsBound() const throw();						// �Ƿ����Դ�ļ�
	bool Parse() throw();								// ���дʷ�����
	bool Print(string fileName) const throw();			// ������ļ�
	void Print(ostream &output) const throw();			// �������
	void ResetTokenPos() throw();						// ���÷���vector�ĵ�����token_vector_��λ��
	bool GetNextToken(Token &token) throw();			// ��ȡ��һ���ţ��ɹ��򷵻�true����������β�򷵻�false
	string GetLine(size_t line_no) throw();
	vector<string> getStringTable() const throw();		// ��ó����ַ�����
private:
	LexicalAnalyzer(const LexicalAnalyzer&) throw();	// ����private�ĸ��ƹ��캯����������ʵ��==>���ø���
	char getNextChar(bool skipSpace = false) throw();	// �õ���һ���ַ���������ʾ�Ƿ��������ַ�
	void ParseCurrentToken(Token &token, char &ch) throw(LexException);			// �����Ѷ����ĵ�ǰ�ַ�������������token����������һ���ַ�
	// ����ļ���handle��������ȷ����token֮�󣬾�Ҫ������һ���ַ�
	void SingleLineCommentHandle(Token &token, char &ch) throw();				// ����ע��
	void BlockCommentHandle		(Token &token, char &ch) throw(LexException);	// ����ע��
	void DigitHandle			(Token &token, char &ch) throw(LexException);	// �����޷�����������
	void LetterHandle			(Token &token, char &ch) throw();				// ������ĸ
	void StringHandle			(Token &token, char &ch) throw(LexException);	// �����ַ�������
	void CharHandle				(Token &token, char &ch) throw(LexException);	// �����ַ�����
	void ColonHandle			(Token &token, char &ch) throw();				// ����ð��
	void LessthanHandle			(Token &token, char &ch) throw();				// ����С�ں�
	void GreaterthanHandle		(Token &token, char &ch) throw();				// ������ں�
	void PlusHandle				(Token &token, char &ch) throw(LexException);	// ����Ӻ�(����������˫������)��ע�⣬�����tokenҪ�洢��һ��ʶ����ĵ���
	void MinusHandle			(Token &token, char &ch) throw(LexException);	// �������(����������˫������)��ע�⣬�����tokenҪ�洢��һ��ʶ����ĵ���

	ifstream srcfile_;							// ������ļ���
	int currentline_;							// ��ǰ������кţ���1��ʼ������
	vector<Token> token_vector_;				// ���Token������
	vector<Token>::const_iterator token_iter_;	// ����token_vector_�ĵ�����
	vector<string> code_lines_;					// ������
	set<string> string_set;						// �����ַ�����

	static const char escape_sequence[];
};

#endif