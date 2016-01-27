//#pragma warning (disable : 4996)	// freopen

#include <iostream>
#include <string>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "TokenTable.h"

using namespace std;

int main(int argc, char *argv[])
{
	bool lex_legitimate = false;
	bool syntax_legitimate = false;
	const string kCodeFileName = "example.cpp";
	const string kTokenFileName = "example_token.txt";
	const string kTokenTableFileName = "example_tokentable.txt";
	const string kSyntaxFileName = "example_syntax.txt";

	std::ofstream syntex_ostream(kSyntaxFileName);

	// �ʷ�����
	LexicalAnalyzer lex_analyzer(kCodeFileName);
	if(!lex_analyzer.IsBound())
	{
		cout << "Cannot open source file " << kCodeFileName << endl;
		return EXIT_FAILURE;
	}
	lex_legitimate = lex_analyzer.Parse();		// ���дʷ�����������״̬
	lex_analyzer.Print(kTokenFileName);			// ������ļ�
	if(!lex_legitimate)							// ������
	{
		//cout << "�ʷ���������" << endl;
		return -1;
	}

	cout << "Token list:\n";
	lex_analyzer.Print(cout);		// ��������б�

	vector<string> stringtable = lex_analyzer.getStringTable();		// �ַ�����
	for(vector<string>::const_iterator iter = stringtable.begin();	// ����ַ�����
		iter != stringtable.end(); ++iter)
	{
		cout << *iter << endl;
	}

	// �﷨����
	TokenTable tokentable;			// ���ű�
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, tokentable);
	syntax_legitimate = syntax_analyzer.Parse();	// �����﷨����������״̬

	tokentable.Print(kTokenTableFileName);		// ������ű�
	if(!syntax_legitimate)			// ������
	{
		//cout << "�﷨��������" << endl;
		return -1;
	}
	cout << "\nToken table:\n" << tokentable.toString();

	return 0;
}