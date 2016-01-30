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
	bool lex_legitimate = false;
	bool syntax_legitimate = false;
	bool assemble_legitimate = false;

	const string kCodeFileName = "test05.txt";
	const string kTokenFileName = "example_token.txt";
	const string kTokenTableFileName = "example_tokentable.txt";
	const string kStringTableFileName = "example_stringtable.txt";
	const string kSyntaxFileName = "example_syntax.txt";
	const string kQuaternaryCodeFileName = "example_quaternary.txt";
	const string kAssemblyCodeFileName = "example_assembly.asm";

	//Quaternary q(Quaternary::ADD, Quaternary::IMMEDIATE_OPERAND, 3, Quaternary::CONSTANT_OPERAND, 5, Quaternary::VARIABLE_OPERAND, 7);

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

	// 过渡
	vector<string> stringtable = lex_analyzer.getStringTable();						// 字符串表
	TokenTable tokentable;															// 符号表
	vector<Quaternary> quaternarytable;												// 四元式表

	// 语法分析
	// 用词法分析器、符号表、字符串表和四元式表对语法分析器进行初始化
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, stringtable, tokentable, quaternarytable);
	syntax_legitimate = syntax_analyzer.Parse();									// 进行语法分析并返回状态
	syntax_analyzer.Print(kSyntaxFileName);											// 输出语法分析过程
	tokentable.Print(kTokenTableFileName);											// 输出符号表
	PrintStringVector(stringtable, kStringTableFileName);							// 输出字符串表
	PrintQuaternaryVector(quaternarytable, tokentable, kQuaternaryCodeFileName);	// 输出四元式
	PrintQuaternaryVector(quaternarytable, tokentable, cout);						// 输出四元式
	if(!syntax_legitimate)// 出错提示
	{
		cout << "语法分析出错！" << endl;
		return -1;
	}

	// 用四元式表、符号表、字符串表初始化汇编器
	AssemblyMaker assembly_maker(quaternarytable, tokentable, stringtable);
	assemble_legitimate = assembly_maker.Assemble();
	assembly_maker.Print(kAssemblyCodeFileName);
	assembly_maker.Print(cout);
	
	// 这里好像不应该有错
	if(!assemble_legitimate)
	{
		cout << "汇编过程出错！" << endl;
		return -1;
	}

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

	//Quaternary q(Quaternary::ADD, Quaternary::IMMEDIATE_OPERAND, 3, Quaternary::CONSTANT_OPERAND, 5, Quaternary::VARIABLE_OPERAND, 7);

	// 词法分析
	LexicalAnalyzer lex_analyzer(kCodeFileName);
	if(!lex_analyzer.IsBound())
	{
		cout << "Cannot open source file " << kCodeFileName << endl;
		return EXIT_FAILURE;
	}
	lex_legitimate = lex_analyzer.Parse();	// 进行词法分析并返回状态
	lex_analyzer.Print(kTokenFileName);		// 输出到文件
	lex_analyzer.Print(cout);				// cout输出token
	if(!lex_legitimate)						// 出错处理
	{
		cout << "词法分析出错！" << endl;
		return -1;
	}

	vector<string> stringtable = lex_analyzer.getStringTable();		// 字符串表
	// 输出字符串表
	cout << "const string list:\n";
	for(vector<string>::const_iterator iter = stringtable.begin();	// 输出字符串表
		iter != stringtable.end(); ++iter)
	{
		cout << *iter << endl;
	}

	// 语法分析
	TokenTable tokentable;			// 符号表

	// 用词法分析器、符号表、字符串表和四元式表对语法分析器进行初始化
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, tokentable, stringtable);
	syntax_legitimate = syntax_analyzer.Parse();// 进行语法分析并返回状态
	syntax_analyzer.Print(kSyntaxFileName);		// 输出语法分析过程
	tokentable.Print(kTokenTableFileName);		// 输出符号表
	tokentable.Print(cout);						// cout输出符号表
	if(!syntax_legitimate)			// 出错处理
	{
		cout << "语法分析出错！" << endl;
		return -1;
	}

	return 0;
}
*/