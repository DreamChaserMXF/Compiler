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

	// 词法分析
	LexicalAnalyzer lex_analyzer(kCodeFileName);
	if(!lex_analyzer.IsBound())
	{
		cout << "Cannot open source file " << kCodeFileName << endl;
		return EXIT_FAILURE;
	}
	lex_legitimate = lex_analyzer.Parse();		// 进行词法分析并返回状态
	lex_analyzer.Print(kTokenFileName);			// 输出到文件
	if(!lex_legitimate)							// 出错处理
	{
		//cout << "词法分析出错！" << endl;
		return -1;
	}

	cout << "Token list:\n";
	lex_analyzer.Print(cout);		// 输出单词列表

	vector<string> stringtable = lex_analyzer.getStringTable();		// 字符串表
	for(vector<string>::const_iterator iter = stringtable.begin();	// 输出字符串表
		iter != stringtable.end(); ++iter)
	{
		cout << *iter << endl;
	}

	// 语法分析
	TokenTable tokentable;			// 符号表
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, tokentable);
	syntax_legitimate = syntax_analyzer.Parse();	// 进行语法分析并返回状态

	tokentable.Print(kTokenTableFileName);		// 输出符号表
	if(!syntax_legitimate)			// 出错处理
	{
		//cout << "语法分析出错！" << endl;
		return -1;
	}
	cout << "\nToken table:\n" << tokentable.toString();

	return 0;
}