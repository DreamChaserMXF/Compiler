//#pragma warning (disable : 4996)	// freopen

#include <iostream>
#include <string>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "TokenTable.h"
#include "Quaternary.h"
#include "AssemblyMaker.h"

using namespace std;


void PrintQuaternaryVector(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, ostream &out) throw()
{
	for(vector<Quaternary>::const_iterator iter = quaternarytable.begin();
		iter != quaternarytable.end(); ++iter)
	{
		out.width(8);
		//out.setf(ios::left);
		out << Quaternary::OPCodeString[iter->op_] << '\t';
		switch(iter->type1_)
		{
		case Quaternary::IMMEDIATE_OPERAND:
			out << iter->src1_;
			break;
		case Quaternary::CONSTANT_OPERAND:
			out << tokentable.at(iter->src1_).name_ << "(" << tokentable.at(iter->src1_).value_ << ")@" << iter->src1_;
			break;
		case Quaternary::VARIABLE_OPERAND:
			out << tokentable.at(iter->src1_).name_ << "@" << iter->src1_;
			break;
		case Quaternary::TEMPORARY_OPERAND:
			out << "TEMP@" << iter->src1_;
			break;
		case Quaternary::LABEL_OPERAND:
			out << "LABEL@" << iter->src1_;
			break;
		case Quaternary::PROC_FUNC_INDEX:
			out << tokentable.at(iter->src1_).name_ << "@" << iter->src1_;
			break;
		case Quaternary::PARANUM_OPERAND:
			out << iter->src1_;
			break;
		default:
			out << iter->src1_;
			break;
		}
		out << '\t';
		switch(iter->type2_)
		{
		case Quaternary::IMMEDIATE_OPERAND:
			out << iter->src2_;
			break;
		case Quaternary::CONSTANT_OPERAND:
			out << tokentable.at(iter->src2_).name_ << "(" << tokentable.at(iter->src2_).value_ << ")@" << iter->src2_;
			break;
		case Quaternary::VARIABLE_OPERAND:
			out << tokentable.at(iter->src2_).name_ << "@" << iter->src2_;
			break;
		case Quaternary::TEMPORARY_OPERAND:
			out << "TEMP@" << iter->src2_;
			break;
		case Quaternary::LABEL_OPERAND:
			out << "LABEL@" << iter->src2_;
			break;
		case Quaternary::PROC_FUNC_INDEX:
			out << tokentable.at(iter->src2_).name_ << "@" << iter->src2_;
			break;
		case Quaternary::PARANUM_OPERAND:
			out << iter->src2_;
			break;
		default:
			out << iter->src2_;
			break;
		}
		out << '\t';
		switch(iter->type3_)
		{
		case Quaternary::IMMEDIATE_OPERAND:
			out << iter->dst_;
			break;
		case Quaternary::CONSTANT_OPERAND:
			out << tokentable.at(iter->dst_).name_ << "(" << tokentable.at(iter->dst_).value_ << ")@" << iter->dst_;
			break;
		case Quaternary::VARIABLE_OPERAND:
			out << tokentable.at(iter->dst_).name_ << "@" << iter->dst_;
			break;
		case Quaternary::TEMPORARY_OPERAND:
			out << "TEMP@" << iter->dst_;
			break;
		case Quaternary::LABEL_OPERAND:
			out << "LABEL@" << iter->dst_;
			break;
		case Quaternary::PROC_FUNC_INDEX:
			out << tokentable.at(iter->dst_).name_ << "@" << iter->dst_;
			break;
		case Quaternary::PARANUM_OPERAND:
			out << iter->dst_;
			break;
		default:
			out << iter->dst_;
			break;
		}
		out << endl;
	}
}

bool PrintQuaternaryVector(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, const string &filename) throw()
{
	ofstream out(filename);
	if(!out.is_open())
	{
		return false;
	}
	PrintQuaternaryVector(quaternarytable, tokentable, out);
	out.close();
	return true;
}

int main(int argc, char *argv[])
{
	bool lex_legitimate = false;
	bool syntax_legitimate = false;
	bool assemble_legitimate = false;

	const string kCodeFileName = "example.cpp";
	const string kTokenFileName = "example_token.txt";
	const string kTokenTableFileName = "example_tokentable.txt";
	const string kSyntaxFileName = "example_syntax.txt";
	const string kQuaternaryCodeFileName = "example_quaternary.txt";
	const string kAssemblyCodeFileName = "example_assembly.asm";

	//Quaternary q(Quaternary::ADD, Quaternary::IMMEDIATE_OPERAND, 3, Quaternary::CONSTANT_OPERAND, 5, Quaternary::VARIABLE_OPERAND, 7);

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

	// ����
	vector<string> stringtable = lex_analyzer.getStringTable();		// �ַ�����
	TokenTable tokentable;											// ���ű�
	vector<Quaternary> quaternarytable;								// ��Ԫʽ��

	// �﷨����
	// �ôʷ������������ű��ַ��������Ԫʽ����﷨���������г�ʼ��
	SyntaxAnalyzer syntax_analyzer(lex_analyzer, stringtable, tokentable, quaternarytable);
	syntax_legitimate = syntax_analyzer.Parse();// �����﷨����������״̬
	syntax_analyzer.Print(kSyntaxFileName);		// ����﷨��������
	tokentable.Print(kTokenTableFileName);		// ������ű�
	PrintQuaternaryVector(quaternarytable, tokentable, kQuaternaryCodeFileName);		// �����Ԫʽ
	PrintQuaternaryVector(quaternarytable, tokentable, cout);		// �����Ԫʽ
	if(!syntax_legitimate)						// ������ʾ
	{
		cout << "�﷨��������" << endl;
		return -1;
	}

	// ����Ԫʽ�����ű��ַ������ʼ�������
	AssemblyMaker assembly_maker(quaternarytable, tokentable, stringtable);
	assemble_legitimate = assembly_maker.Assemble();
	assembly_maker.Print(kAssemblyCodeFileName);
	assembly_maker.Print(cout);
	
	// �������Ӧ���д�
	if(!assemble_legitimate)
	{
		cout << "�����̳���" << endl;
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