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
	// 分析器的状态
	bool lex_legitimate = false;
	bool syntax_legitimate = false;
	bool assemble_legitimate = false;

	// 输入输出文件名
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
	// 删除原有的文件
	string delete_command = "del ";	// 当要删除的文件不存在时，@并没有关闭错误提示
	system((delete_command + kTokenFileName).c_str());
	system((delete_command + kTokenTableFileName).c_str());
	system((delete_command + kStringTableFileName).c_str());
	system((delete_command + kSyntaxFileName).c_str());
	system((delete_command + kQuaternaryCodeFileName).c_str());
	system((delete_command + kAssemblyCodeFileName).c_str());

	// 词法分析
	LexicalAnalyzer lex_analyzer(kCodeFileName);
	if(!lex_analyzer.IsBound())
	{
		cout << "Cannot open source file " << kCodeFileName << endl;
		return EXIT_FAILURE;
	}
	lex_legitimate = lex_analyzer.Parse();	// 进行词法分析并返回状态
	lex_analyzer.Print(kTokenFileName);		// 输出到文件
	if(!lex_legitimate)						// 出错提示
	{
		cout << "词法分析出错！" << endl;
		return -1;
	}

	// 得到字符串表，建立符号表和四元式表
	vector<string> stringtable = lex_analyzer.getStringTable();						// 字符串表
	TokenTable tokentable;															// 符号表
	vector<Quaternary> quaternarytable;												// 四元式表

	// 语法分析
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, 
		stringtable, tokentable, quaternarytable);// 用词法分析器、符号表、字符串表和四元式表对语法分析器进行初始化
	syntax_legitimate = syntax_analyzer.Parse();									// 进行语法分析并返回状态
	syntax_analyzer.Print(kSyntaxFileName);											// 输出语法分析过程
	tokentable.Print(kTokenTableFileName);											// 输出符号表
	PrintStringVector(stringtable, kStringTableFileName);							// 输出字符串表
	PrintQuaternaryVector(quaternarytable, tokentable, kQuaternaryCodeFileName);	// 输出四元式
//	PrintQuaternaryVector(quaternarytable, tokentable, cout);						// 输出四元式
	if(!syntax_legitimate)// 出错提示
	{
		cout << "语法分析出错！" << endl;
		return -1;
	}

	// 目标代码生成
	// 用四元式表、符号表、字符串表初始化汇编器
	AssemblyMaker assembly_maker(quaternarytable, tokentable, stringtable);
	assemble_legitimate = assembly_maker.Assemble();
	assembly_maker.Print(kAssemblyCodeFileName);
	//出错检查
	if(!assemble_legitimate)// 这里好像不应该有错
	{
		cout << "汇编过程出错！" << endl;
		return -1;
	}

	// 汇编过程
	// 先确定不带后缀的汇编文件名
	string object_path_file = kAssemblyCodeFileName.substr(0, kAssemblyCodeFileName.find_last_of('.'));
	string object_single_file = object_path_file.substr(object_path_file.find('\\') + 1, object_path_file.length() - object_path_file.find('\\') - 1);
	// 删除原有文件
	system((delete_command + object_single_file + ".obj").c_str());
	system((delete_command + object_single_file + ".exe").c_str());
	// 汇编
	string ml_command = "masm32\\bin\\ml.exe /c /coff ";
	string link_command = "masm32\\bin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF ";
	system((ml_command + object_path_file + ".asm").c_str());
	system((link_command + object_single_file + ".obj").c_str());
	// 运行
	system((object_single_file + ".exe").c_str());
	//system("masm32\\bin\\ml.exe /c /coff TestCase/result6_assembly.asm TestCase/result60_assembly.obj");	// 这里的执行应用程序的目录一定要用右斜杠
	//system("masm32\\bin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF result6_assembly.obj");
	//system("result6_assembly.exe");
	return 0;
}
