//#pragma warning (disable : 4996)	// freopen

#include <iostream>
#include <string>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "TokenTable.h"
#include "Quaternary.h"
#include "AssemblyMaker.h"

#include "Print.h"

using namespace std;

int main(int argc, char *argv[])
{
	// ��������״̬
	bool lex_legitimate = false;
	bool syntax_legitimate = false;
	bool assemble_legitimate = false;

	// ��������ļ���
	const string kCodeFileName = "TestCase/05_array.cpp";
	const string kTokenFileName = "TestCase/example_token.txt";
	const string kTokenTableFileName = "TestCase/example_tokentable.txt";
	const string kStringTableFileName = "TestCase/example_stringtable.txt";
	const string kSyntaxFileName = "TestCase/example_syntax.txt";
	const string kQuaternaryCodeFileName = "TestCase/example_quaternary.txt";
	const string kAssemblyCodeFileName = "TestCase/example_assembly.asm";

	// �ʷ�����
	LexicalAnalyzer lex_analyzer(kCodeFileName);
	if(!lex_analyzer.IsBound())
	{
		cout << "Cannot open source file " << kCodeFileName << endl;
		return EXIT_FAILURE;
	}
	lex_legitimate = lex_analyzer.Parse();	// ���дʷ�����������״̬
	lex_analyzer.Print(kTokenFileName);		// ������ļ�
	if(!lex_legitimate)						// ������ʾ
	{
		cout << "�ʷ���������" << endl;
		return -1;
	}

	// �õ��ַ������������ű����Ԫʽ��
	vector<string> stringtable = lex_analyzer.getStringTable();						// �ַ�����
	TokenTable tokentable;															// ���ű�
	vector<Quaternary> quaternarytable;												// ��Ԫʽ��

	// �﷨����
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, 
		stringtable, tokentable, quaternarytable);// �ôʷ������������ű��ַ��������Ԫʽ����﷨���������г�ʼ��
	syntax_legitimate = syntax_analyzer.Parse();									// �����﷨����������״̬
	syntax_analyzer.Print(kSyntaxFileName);											// ����﷨��������
	tokentable.Print(kTokenTableFileName);											// ������ű�
	PrintStringVector(stringtable, kStringTableFileName);							// ����ַ�����
	PrintQuaternaryVector(quaternarytable, tokentable, kQuaternaryCodeFileName);	// �����Ԫʽ
//	PrintQuaternaryVector(quaternarytable, tokentable, cout);						// �����Ԫʽ
	if(!syntax_legitimate)// ������ʾ
	{
		cout << "�﷨��������" << endl;
		return -1;
	}

	// Ŀ���������
	// ����Ԫʽ�����ű��ַ������ʼ�������
	AssemblyMaker assembly_maker(quaternarytable, tokentable, stringtable);
	assemble_legitimate = assembly_maker.Assemble();
	assembly_maker.Print(kAssemblyCodeFileName);
	//������
	if(!assemble_legitimate)// �������Ӧ���д�
	{
		cout << "�����̳���" << endl;
		return -1;
	}

	// ������
	system("masm32\\bin\\ml.exe /c /coff ./TestCase/example_assembly.asm");	// �����ִ��Ӧ�ó����Ŀ¼һ��Ҫ����б��
	system("masm32\\bin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF example_assembly.obj");
	system("example_assembly.exe");
	return 0;
}
/*

int main(int argc, char *argv[])
{
	bool lex_legitimate = false;
	bool syntax_legitimate = false;
	const string kCodeFileName = "example.cpp";
	const string kTokenFileName = "example_token.txt";
	const string kTokenTableFileName = "example_tokentable.txt";
	const string kSyntaxFileName = "example_syntax.txt";

	//Quaternary q(Quaternary::ADD, Quaternary::IMMEDIATE_ADDRESSING, 3, Quaternary::CONSTANT_ADDRESSING, 5, Quaternary::VARIABLE_ADDRESSING, 7);

	// �ʷ�����
	LexicalAnalyzer lex_analyzer(kCodeFileName);
	if(!lex_analyzer.IsBound())
	{
		cout << "Cannot open source file " << kCodeFileName << endl;
		return EXIT_FAILURE;
	}
	lex_legitimate = lex_analyzer.Parse();	// ���дʷ�����������״̬
	lex_analyzer.Print(kTokenFileName);		// ������ļ�
	lex_analyzer.Print(cout);				// cout���token
	if(!lex_legitimate)						// ������
	{
		cout << "�ʷ���������" << endl;
		return -1;
	}

	vector<string> stringtable = lex_analyzer.getStringTable();		// �ַ�����
	// ����ַ�����
	cout << "const string list:\n";
	for(vector<string>::const_iterator iter = stringtable.begin();	// ����ַ�����
		iter != stringtable.end(); ++iter)
	{
		cout << *iter << endl;
	}

	// �﷨����
	TokenTable tokentable;			// ���ű�

	// �ôʷ������������ű��ַ��������Ԫʽ����﷨���������г�ʼ��
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, tokentable, stringtable);
	syntax_legitimate = syntax_analyzer.Parse();// �����﷨����������״̬
	syntax_analyzer.Print(kSyntaxFileName);		// ����﷨��������
	tokentable.Print(kTokenTableFileName);		// ������ű�
	tokentable.Print(cout);						// cout������ű�
	if(!syntax_legitimate)			// ������
	{
		cout << "�﷨��������" << endl;
		return -1;
	}

	return 0;
}
*/