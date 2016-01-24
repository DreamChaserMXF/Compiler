//#pragma warning (disable : 4996)	// freopen

#include <iostream>
#include <string>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "TokenTable.h"

using namespace std;

int main(int argc, char *argv[])
{
	bool lexLegitimate = false;
	bool syntaxLegitimate = false;
	string codeFileName = "example.cpp";
	string tokenFileName = "example_token.txt";
	string tokenTableFileName = "example_tokentable.txt";
	string syntax_info_buffer_file = "example_syntax.txt";

	std::ofstream syntex_ostream(syntax_info_buffer_file);

	// �ʷ�����
	LexicalAnalyzer lexAnalyzer(codeFileName);
	lexLegitimate = lexAnalyzer.Parse();		// ���дʷ�����������״̬
	lexAnalyzer.Print(tokenFileName);			// ������ļ�
//	lexAnalyzer.Print(cout);					// �������
	if(!lexLegitimate)				// ������
	{
		//cout << "�ʷ���������" << endl;
		return -1;
	}
	cout << "Token list:\n";
	lexAnalyzer.Print(cout);		// ��������б�
	vector<string> stringTable = lexAnalyzer.getStringTable();		// �ַ�����
	
	for(vector<string>::const_iterator iter = stringTable.begin(); iter != stringTable.end(); ++iter)
	{
		cout << *iter << endl;
	}

	// �﷨����
	TokenTable tokenTable;			// ���ű�
	SyntaxAnalyzer syntaxAnalyzer(lexAnalyzer, tokenTable, syntex_ostream);
	syntaxLegitimate = syntaxAnalyzer.Parse();	// �����﷨����������״̬
	tokenTable.Print(tokenTableFileName);		// ������ű�
	if(!syntaxLegitimate)			// ������
	{
		//cout << "�﷨��������" << endl;
		return -1;
	}
	cout << "\nToken table:\n" << tokenTable.toString();

	return 0;
}