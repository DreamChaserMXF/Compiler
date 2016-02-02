#include "Print.h"

void PrintOperand(Quaternary::AddressingMethod type, int value, const TokenTable &tokentable, ostream &out, int space_width) throw()
{
	std::ostringstream buffer;
	switch(type)
	{
	case Quaternary::IMMEDIATE_ADDRESSING:
		buffer << value;
		break;
	//case Quaternary::CONSTANT_ADDRESSING:
	//	buffer << tokentable.at(value).name_ << "<" << tokentable.at(value).value_ << ">#" << value;
	//	break;
	case Quaternary::STRING_ADDRESSING:
		buffer << "_string#" << value;
		break;
	case Quaternary::VARIABLE_ADDRESSING:
		buffer << tokentable.at(value).name_ << "#" << value;
		break;
	case Quaternary::ARRAY_ADDRESSING:
		buffer << tokentable.at(value).name_ << "#" << value;
		break;
	case Quaternary::TEMPORARY_ADDRESSING:
		buffer << "_temp#" << value;
		break;
	//case Quaternary::LABEL_ADDRESSING:
	//	buffer << "_label#" << value;
	//	break;
	//case Quaternary::PROC_FUNC_INDEX:
	//	buffer << tokentable.at(value).name_ << "#" << value;
	//	break;
	//case Quaternary::PARANUM_ADDRESSING:
	//	buffer << "PARA#" << value;
	//	break;
	default:
		buffer << value;
		break;
	}
	out.width(space_width);
	out.setf(ios::right);
	out << buffer.str();
}

void PrintQuaternaryVector(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, ostream &out) throw()
{
	for(vector<Quaternary>::const_iterator iter = quaternarytable.begin();
		iter != quaternarytable.end(); ++iter)
	{
		// 输出序号
		out.width(3);
		out.setf(ios::right);
		out << distance(quaternarytable.begin(), iter) << '\t';
		// 输出操作符
		out.width(9);
		out.setf(ios::right);
		out << Quaternary::OPCodeString[iter->op_];
		// 输出操作数
		PrintOperand(iter->method1_, iter->src1_, tokentable, out);
		PrintOperand(iter->method2_, iter->src2_, tokentable, out);
		PrintOperand(iter->method3_, iter->dst_, tokentable, out);
		// 输出换行
		out << '\n';
	}
	out.flush();
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

void PrintStringVector(const vector<string> &stringtable, ostream &out) throw()
{
	for(vector<string>::const_iterator iter = stringtable.begin();
		iter != stringtable.end(); ++iter)
	{
		//out.width(3);
		//out.setf(ios::right);
		out << distance(stringtable.begin(), iter) << '\t' << *iter << '\n';
	}
	out.flush();
}

bool PrintStringVector(const vector<string> &stringtable, const string &filename) throw()
{
	ofstream out(filename);
	if(!out.is_open())
	{
		return false;
	}
	PrintStringVector(stringtable, out);
	out.close();
	return true;
}