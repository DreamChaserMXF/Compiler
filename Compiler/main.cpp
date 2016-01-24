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

	// 词法分析
	LexicalAnalyzer lexAnalyzer(codeFileName);
	lexLegitimate = lexAnalyzer.Parse();		// 进行词法分析并返回状态
	lexAnalyzer.Print(tokenFileName);			// 输出到文件
//	lexAnalyzer.Print(cout);					// 输出到流
	if(!lexLegitimate)				// 出错处理
	{
		//cout << "词法分析出错！" << endl;
		return -1;
	}
	cout << "Token list:\n";
	lexAnalyzer.Print(cout);		// 输出单词列表
	vector<string> stringTable = lexAnalyzer.getStringTable();		// 字符串表
	
	for(vector<string>::const_iterator iter = stringTable.begin(); iter != stringTable.end(); ++iter)
	{
		cout << *iter << endl;
	}

	// 语法分析
	TokenTable tokenTable;			// 符号表
	SyntaxAnalyzer syntaxAnalyzer(lexAnalyzer, tokenTable, syntex_ostream);
	syntaxLegitimate = syntaxAnalyzer.Parse();	// 进行语法分析并返回状态
	tokenTable.Print(tokenTableFileName);		// 输出符号表
	if(!syntaxLegitimate)			// 出错处理
	{
		//cout << "语法分析出错！" << endl;
		return -1;
	}
	cout << "\nToken table:\n" << tokenTable.toString();

	return 0;
}