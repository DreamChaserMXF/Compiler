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
	//const string kCodeFileName = "TestCase\\example.cpp";
	const string kCodeFileName = "TestCase\\00_generaltest.cpp";
	//const string kCodeFileName = "TestCase\\01_write.cpp";
	//const string kCodeFileName = "TestCase\\02_read.cpp";
	//const string kCodeFileName = "TestCase\\03_arithmetic.cpp";
	//const string kCodeFileName = "TestCase\\04_condition.cpp";
	//const string kCodeFileName = "TestCase\\05_array.cpp";
	//const string kCodeFileName = "TestCase\\06_loop.cpp";
	//const string kCodeFileName = "TestCase\\07_procedurecall.cpp";
	//const string kCodeFileName = "TestCase\\08_functioncall_factorial.cpp";
	//const string kCodeFileName = "TestCase\\09_functioncall_statement_extension.cpp";
	//const string kCodeFileName = "TestCase\\10_procedure_reference_parameter.cpp";
	//const string kCodeFileName = "TestCase\\11_function_reference_parameter.cpp";
	const string kTokenFileName = "TestCase\\result1_tokenlist.txt";
	const string kTokenTableFileName = "TestCase\\result2_tokentable.txt";
	const string kStringTableFileName = "TestCase\\result3_stringtable.txt";
	const string kSyntaxFileName = "TestCase\\result4_syntaxprocess.txt";
	const string kQuaternaryCodeFileName = "TestCase\\result5_midcode.txt";
	const string kAssemblyCodeFileName = "TestCase\\result_final_assembly.asm";
	// ɾ��ԭ�е��ļ�
	string delete_command = "del ";	// ��Ҫɾ�����ļ�������ʱ��@��û�йرմ�����ʾ
	system((delete_command + kTokenFileName).c_str());
	system((delete_command + kTokenTableFileName).c_str());
	system((delete_command + kStringTableFileName).c_str());
	system((delete_command + kSyntaxFileName).c_str());
	system((delete_command + kQuaternaryCodeFileName).c_str());
	system((delete_command + kAssemblyCodeFileName).c_str());

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
	// ��ȷ��������׺�Ļ���ļ���
	string object_path_file = kAssemblyCodeFileName.substr(0, kAssemblyCodeFileName.find_last_of('.'));
	string object_single_file = object_path_file.substr(object_path_file.find('\\') + 1, object_path_file.length() - object_path_file.find('\\') - 1);
	// ɾ��ԭ���ļ�
	system((delete_command + object_single_file + ".obj").c_str());
	system((delete_command + object_single_file + ".exe").c_str());
	// ���
	string ml_command = "masm32\\bin\\ml.exe /c /coff ";
	string link_command = "masm32\\bin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF ";
	system((ml_command + object_path_file + ".asm").c_str());
	system((link_command + object_single_file + ".obj").c_str());
	// ����
	system((object_single_file + ".exe").c_str());
	//system("masm32\\bin\\ml.exe /c /coff TestCase/result6_assembly.asm TestCase/result60_assembly.obj");	// �����ִ��Ӧ�ó����Ŀ¼һ��Ҫ����б��
	//system("masm32\\bin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF result6_assembly.obj");
	//system("result6_assembly.exe");
	return 0;
}
