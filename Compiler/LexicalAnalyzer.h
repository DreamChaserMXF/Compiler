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

// �ʷ�������
// �����׳��ֵ�BUG��whileѭ���У���ch='\0'���᲻������ѭ��
class LexicalAnalyzer
{
public:
	LexicalAnalyzer(const string &srcFileName);
	bool Parse();							// ���дʷ�����
	void Print(string fileName) const;		// ������ļ�
	void Print(ostream &output) const;		// �������
	void ResetTokenPos();					// ���÷���vector�ĵ�����tokenVector��λ��
	bool GetNextToken(Token &token);		// ��ȡ��һ���ţ��ɹ��򷵻�true����������β�򷵻�false
	vector<string> getStringTable();		// ��ó����ַ�����
private:
	LexicalAnalyzer(const LexicalAnalyzer&);		// ����private�ĸ��ƹ��캯����������ʵ��==>���ø���
	char getNextChar(bool skipSpace = false);		// �õ���һ���ַ���������ʾ�Ƿ��������ַ�
	void parseCurrentToken(Token &token, char &ch);	// �����Ѷ����ĵ�ǰ�ַ�������������token����������һ���ַ�
	// ����ļ���handle��������ȷ����token֮�󣬾�Ҫ������һ���ַ�
	void singleLineCommentHandle(Token &token, char &ch);			// ����ע��
	void blockCommentHandle		(Token &token, char &ch);			// ����ע��
	void digitHandle			(Token &token, char &ch);	// �����޷�����������
	void letterHandle			(Token &token, char &ch);	// ������ĸ
	void stringHandle			(Token &token, char &ch);	// �����ַ�������
	void charHandle				(Token &token, char &ch);	// �����ַ�����
	void colonHandle			(Token &token, char &ch);	// ����ð��
	void ltHandle				(Token &token, char &ch);	// ����С�ں�
	void gtHandle				(Token &token, char &ch);	// ������ں�
	void plusHandle				(Token &token, char &ch);	// ����Ӻ�(����������˫������)��ע�⣬�����tokenҪ�洢��һ��ʶ����ĵ���
	void minusHandle			(Token &token, char &ch);	// �������(����������˫������)��ע�⣬�����tokenҪ�洢��һ��ʶ����ĵ���

	ifstream srcFile;						// ������ļ���
	int currentLine;						// ��ǰ������к�
	vector<Token> tokenVector;				// ���Token������
	vector<Token>::const_iterator tokenIter;
	set<string> stringSet;				// �����ַ�����
};

#endif